use crate::{
    error::{self, Error},
    maybe_trace::MaybeTrace,
    micropython::{
        gc::{self, GcBox},
        obj::Obj,
    },
    ui::{
        component::{
            base::AttachType::{self, Swipe},
            Component, Event, EventCtx, FlowMsg, SwipeDetect, SwipeDetectMsg, SwipeDirection,
        },
        display::Color,
        event::{SwipeEvent, TouchEvent},
        flow::{base::Decision, FlowController},
        geometry::Rect,
        layout::obj::ObjComponent,
        shape::{render_on_display, ConcreteRenderer, Renderer, ScopedRenderer},
        util::animation_disabled,
    },
};

use super::{base::FlowState, Swipable};

use heapless::Vec;

/// Component-like proto-object-safe trait.
///
/// This copies the Component interface, but it is parametrized by a concrete
/// Renderer type, making it object-safe.
pub trait FlowComponentTrait<'s, R: Renderer<'s>>: Swipable {
    fn place(&mut self, bounds: Rect) -> Rect;
    fn event(&mut self, ctx: &mut EventCtx, event: Event) -> Option<FlowMsg>;
    fn render(&'s self, target: &mut R);

    #[cfg(feature = "ui_debug")]
    fn trace(&self, t: &mut dyn crate::trace::Tracer);
}

/// FlowComponentTrait implementation for Components.
///
/// Components implementing FlowComponentTrait must:
/// * also implement Swipable, required by FlowComponentTrait,
/// * use FlowMsg as their Msg type,
/// * implement MaybeTrace to be able to conform to ObjComponent.
impl<'s, R, C> FlowComponentTrait<'s, R> for C
where
    C: Component<Msg = FlowMsg> + MaybeTrace + Swipable,
    R: Renderer<'s>,
{
    fn place(&mut self, bounds: Rect) -> Rect {
        <Self as Component>::place(self, bounds)
    }

    fn event(&mut self, ctx: &mut EventCtx, event: Event) -> Option<FlowMsg> {
        <Self as Component>::event(self, ctx, event)
    }

    fn render(&'s self, target: &mut R) {
        <Self as Component>::render(self, target)
    }

    #[cfg(feature = "ui_debug")]
    fn trace(&self, t: &mut dyn crate::trace::Tracer) {
        <Self as crate::trace::Trace>::trace(self, t)
    }
}

/// Shortcut type for the concrete renderer passed into `render()` method.
type RendererImpl<'a, 'alloc, 'env> = ScopedRenderer<'alloc, 'env, ConcreteRenderer<'a, 'alloc>>;

/// Fully object-safe component-like trait for flow components.
///
/// This trait has no generic parameters:
/// * it is instantiated with a concrete Renderer type, and
/// * it requires the `FlowComponentTrait` trait to be implemented for _any_
///   lifetimes.
pub trait FlowComponentDynTrait:
    for<'a, 'alloc, 'env> FlowComponentTrait<'alloc, RendererImpl<'a, 'alloc, 'env>>
{
}

impl<T> FlowComponentDynTrait for T where
    T: for<'a, 'alloc, 'env> FlowComponentTrait<'alloc, RendererImpl<'a, 'alloc, 'env>>
{
}

/// Swipe flow consisting of multiple screens.
///
/// Implements swipe navigation between the states with animated transitions,
/// based on state transitions provided by the FlowState type.
///
/// If a swipe is detected:
/// - currently active component is asked to handle the event,
/// - if it can't then FlowState::handle_swipe is consulted.
pub struct SwipeFlow {
    /// Current state of the flow.
    state: FlowState,
    /// Store of all screens which are part of the flow.
    store: Vec<GcBox<dyn FlowComponentDynTrait>, 12>,
    /// Swipe detector.
    swipe: SwipeDetect,
    /// Swipe allowed
    allow_swipe: bool,
    /// Current internal state
    internal_state: u16,
    /// Internal pages count
    internal_pages: u16,
    /// If triggering swipe by event, make this decision instead of default
    /// after the swipe.
    decision_override: Option<Decision>,
}

impl SwipeFlow {
    pub fn new(initial_state: &'static dyn FlowController) -> Result<Self, error::Error> {
        Ok(Self {
            state: initial_state,
            swipe: SwipeDetect::new(),
            store: Vec::new(),
            allow_swipe: true,
            internal_state: 0,
            internal_pages: 1,
            decision_override: None,
        })
    }

    /// Add a page to the flow.
    ///
    /// Pages must be inserted in the order of the flow state index.
    pub fn with_page(
        mut self,
        state: &'static dyn FlowController,
        page: impl FlowComponentDynTrait + 'static,
    ) -> Result<Self, error::Error> {
        debug_assert!(self.store.len() == state.index());
        let alloc = GcBox::new(page)?;
        let page = gc::coerce!(FlowComponentDynTrait, alloc);
        unwrap!(self.store.push(page));
        Ok(self)
    }

    fn current_page(&self) -> &GcBox<dyn FlowComponentDynTrait> {
        &self.store[self.state.index()]
    }

    fn current_page_mut(&mut self) -> &mut GcBox<dyn FlowComponentDynTrait> {
        &mut self.store[self.state.index()]
    }

    fn goto(&mut self, ctx: &mut EventCtx, attach_type: AttachType) {
        self.swipe = SwipeDetect::new();
        self.allow_swipe = true;

        self.current_page_mut()
            .event(ctx, Event::Attach(attach_type));

        self.internal_pages = self.current_page_mut().get_internal_page_count() as u16;

        match attach_type {
            Swipe(SwipeDirection::Up) => {
                self.internal_state = 0;
            }
            Swipe(SwipeDirection::Down) => {
                self.internal_state = self.internal_pages.saturating_sub(1);
            }
            _ => {}
        }

        ctx.request_paint();
    }

    fn render_state<'s>(&'s self, state: usize, target: &mut RendererImpl<'_, 's, '_>) {
        self.store[state].render(target);
    }

    fn handle_swipe_child(&mut self, _ctx: &mut EventCtx, direction: SwipeDirection) -> Decision {
        self.state.handle_swipe(direction)
    }

    fn handle_event_child(&mut self, ctx: &mut EventCtx, event: Event) -> Decision {
        let msg = self.current_page_mut().event(ctx, event);

        if let Some(msg) = msg {
            self.state.handle_event(msg)
        } else {
            Decision::Nothing
        }
    }

    fn event(&mut self, ctx: &mut EventCtx, event: Event) -> Option<FlowMsg> {
        let mut decision = Decision::Nothing;
        let mut return_transition: AttachType = AttachType::Initial;

        let mut attach = false;

        let e = if self.allow_swipe {
            let page = self.current_page();
            let mut config = page.get_swipe_config();

            self.internal_pages = page.get_internal_page_count() as u16;

            // set up the pager feature of SwipeConfig based on the internal paging state
            config.configure_paging(self.internal_state, self.internal_pages);

            match self.swipe.event(ctx, event, config) {
                Some(SwipeDetectMsg::Trigger(dir)) => {
                    if let Some(override_decision) = self.decision_override.take() {
                        decision = override_decision;
                    } else {
                        decision = self.handle_swipe_child(ctx, dir);
                    }

                    return_transition = AttachType::Swipe(dir);

                    let paging = config.paging_event(dir);
                    if paging != 0 {
                        let new_internal_state = self.internal_state as i16 + paging as i16;
                        self.internal_state =
                            new_internal_state.clamp(0, self.internal_pages as i16 - 1) as u16;
                        decision = Decision::Nothing;
                        attach = true;
                    }
                    Event::Swipe(SwipeEvent::End(dir))
                }
                Some(SwipeDetectMsg::Move(dir, progress)) => {
                    Event::Swipe(SwipeEvent::Move(dir, progress as i16))
                }
                Some(SwipeDetectMsg::Start(_)) => Event::Touch(TouchEvent::TouchAbort),
                None => event,
            }
        } else {
            event
        };

        match decision {
            Decision::Nothing => {
                decision = self.handle_event_child(ctx, e);

                // when doing internal transition, pass attach event to the child after sending
                // swipe end.
                if attach {
                    if let Event::Swipe(SwipeEvent::End(dir)) = e {
                        self.current_page_mut()
                            .event(ctx, Event::Attach(AttachType::Swipe(dir)));
                    }
                }

                if ctx.disable_swipe_requested() {
                    self.swipe.reset();
                    self.allow_swipe = false;
                }
                if ctx.enable_swipe_requested() {
                    self.swipe.reset();
                    self.allow_swipe = true;
                };

                let config = self.current_page().get_swipe_config();

                if let Decision::Transition(_, Swipe(direction)) = decision {
                    if config.is_allowed(direction) {
                        if !animation_disabled() {
                            self.swipe.trigger(ctx, direction, config);
                            self.decision_override = Some(decision);
                            decision = Decision::Nothing;
                        }
                        self.allow_swipe = true;
                    }
                }
            }
            _ => {
                //ignore message, we are already transitioning
                self.current_page_mut().event(ctx, event);
            }
        }

        match decision {
            Decision::Transition(new_state, attach) => {
                self.state = new_state;
                self.goto(ctx, attach);
                None
            }
            Decision::Return(msg) => {
                ctx.set_transition_out(return_transition);
                self.swipe.reset();
                self.allow_swipe = true;
                Some(msg)
            }
            _ => None,
        }
    }
}

/// ObjComponent implementation for SwipeFlow.
///
/// Instead of using the generic `impl ObjComponent for ComponentMsgObj`, we
/// provide our own short-circuit implementation for `SwipeFlow`. This way we
/// can completely avoid implementing `Component`. That also allows us to pass
/// around concrete Renderers instead of having to conform to `Component`'s
/// not-object-safe interface.
///
/// This implementation relies on the fact that swipe components always return
/// `FlowMsg` as their `Component::Msg` (provided by `impl FlowComponentTrait`
/// earlier in this file).
#[cfg(feature = "micropython")]
impl ObjComponent for SwipeFlow {
    fn obj_place(&mut self, bounds: Rect) -> Rect {
        for elem in self.store.iter_mut() {
            elem.place(bounds);
        }
        bounds
    }

    fn obj_event(&mut self, ctx: &mut EventCtx, event: Event) -> Result<Obj, Error> {
        match self.event(ctx, event) {
            None => Ok(Obj::const_none()),
            Some(msg) => msg.try_into(),
        }
    }

    fn obj_paint(&mut self) {
        render_on_display(None, Some(Color::black()), |target| {
            self.render_state(self.state.index(), target);
        });
    }
}

#[cfg(feature = "ui_debug")]
impl crate::trace::Trace for SwipeFlow {
    fn trace(&self, t: &mut dyn crate::trace::Tracer) {
        self.current_page().trace(t)
    }
}
