use crate::{
    error,
    strutil::TString,
    translations::TR,
    ui::{
        button_request::ButtonRequest,
        component::{swipe_detect::SwipeSettings, ButtonRequestExt, ComponentExt, SwipeDirection},
        flow::{
            base::{DecisionBuilder as _, StateChange},
            FlowMsg, FlowState, SwipeFlow,
        },
    },
};

use core::sync::atomic::{AtomicU16, Ordering};

use super::super::{
    component::{
        Frame, FrameMsg, NumberInputDialog, NumberInputDialogMsg, SwipeContent, UpdatableMoreInfo,
        VerticalMenu, VerticalMenuChoiceMsg,
    },
    theme,
};

#[derive(Copy, Clone, PartialEq, Eq)]
pub enum RequestNumber {
    Number,
    Menu,
    Info,
}

impl FlowState for RequestNumber {
    #[inline]
    fn index(&'static self) -> usize {
        *self as usize
    }

    fn handle_swipe(&'static self, direction: SwipeDirection) -> StateChange {
        match (self, direction) {
            (Self::Number, SwipeDirection::Left) => Self::Menu.swipe(direction),
            (Self::Menu, SwipeDirection::Right) => Self::Number.swipe(direction),
            (Self::Info, SwipeDirection::Right) => Self::Menu.swipe(direction),
            _ => self.do_nothing(),
        }
    }

    fn handle_event(&'static self, msg: FlowMsg) -> StateChange {
        match (self, msg) {
            (Self::Number, FlowMsg::Info) => Self::Menu.transit(),
            (Self::Menu, FlowMsg::Choice(0)) => Self::Info.swipe_left(),
            (Self::Menu, FlowMsg::Cancelled) => Self::Number.swipe_right(),
            (Self::Info, FlowMsg::Cancelled) => Self::Menu.transit(),
            (Self::Number, FlowMsg::Choice(n)) => self.return_msg(FlowMsg::Choice(n)),
            _ => self.do_nothing(),
        }
    }
}

static NUM_DISPLAYED: AtomicU16 = AtomicU16::new(0);

pub fn new_request_number(
    title: TString<'static>,
    count: u32,
    min_count: u32,
    max_count: u32,
    description: TString<'static>,
    info_closure: impl Fn(u32) -> TString<'static> + 'static,  // FIXME: lifetime
    br_code: u16,
    br_name: TString<'static>,
) -> Result<SwipeFlow, error::Error>
{
    NUM_DISPLAYED.store(count as u16, Ordering::Relaxed);

    // wrap the closure for obtaining MoreInfo text and call it with NUM_DISPLAYED
    let info_closure = move || {
        let curr_number = NUM_DISPLAYED.load(Ordering::Relaxed);
        info_closure(curr_number as u32)
    };

    let number_input_dialog = NumberInputDialog::new(min_count, max_count, count, description)?;
    let content_number_input = Frame::left_aligned(title, SwipeContent::new(number_input_dialog))
        .with_menu_button()
        .with_footer(TR::instructions__swipe_up.into(), None)
        .with_swipe(SwipeDirection::Up, SwipeSettings::default())
        .with_swipe(SwipeDirection::Left, SwipeSettings::default())
        .map(|msg| match msg {
            FrameMsg::Button(_) => Some(FlowMsg::Info),
            FrameMsg::Content(NumberInputDialogMsg::Changed(n)) => {
                NUM_DISPLAYED.store(n as u16, Ordering::Relaxed);
                None
            }
            FrameMsg::Content(NumberInputDialogMsg::Confirmed(n)) => {
                NUM_DISPLAYED.store(n as u16, Ordering::Relaxed);
                Some(FlowMsg::Choice(n as usize))
            }
        })
        .one_button_request(ButtonRequest::from_num(br_code, br_name));

    let content_menu = Frame::left_aligned(
        TString::empty(),
        VerticalMenu::empty().item(theme::ICON_CHEVRON_RIGHT, TR::buttons__more_info.into()),
    )
    .with_cancel_button()
    .with_swipe(SwipeDirection::Right, SwipeSettings::immediate())
    .map(|msg| match msg {
        FrameMsg::Content(VerticalMenuChoiceMsg::Selected(i)) => Some(FlowMsg::Choice(i)),
        FrameMsg::Button(FlowMsg::Cancelled) => Some(FlowMsg::Cancelled),
        FrameMsg::Button(_) => None,
    });

    let updatable_info = UpdatableMoreInfo::new(info_closure);
    let content_info = Frame::left_aligned(TString::empty(), SwipeContent::new(updatable_info))
        .with_cancel_button()
        .with_swipe(SwipeDirection::Right, SwipeSettings::immediate())
        .map(|msg| match msg {
            FrameMsg::Button(FlowMsg::Cancelled) => Some(FlowMsg::Cancelled),
            _ => None,
        });

    let res = SwipeFlow::new(&RequestNumber::Number)?
        .with_page(&RequestNumber::Number, content_number_input)?
        .with_page(&RequestNumber::Menu, content_menu)?
        .with_page(&RequestNumber::Info, content_info)?;
    Ok(res)
}
