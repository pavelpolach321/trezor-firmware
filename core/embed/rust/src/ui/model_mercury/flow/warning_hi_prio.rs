use crate::{
    error,
    strutil::TString,
    translations::TR,
    ui::{
        component::{
            swipe_detect::SwipeSettings,
            text::paragraphs::{Paragraph, ParagraphSource},
            ComponentExt, SwipeDirection,
        },
        flow::{
            base::{DecisionBuilder as _, StateChange},
            FlowMsg, FlowState, SwipeFlow,
        },
    },
};

use super::super::{
    component::{Frame, FrameMsg, StatusScreen, SwipeContent, VerticalMenu, VerticalMenuChoiceMsg},
    theme,
};

#[derive(Copy, Clone, PartialEq, Eq)]
pub enum WarningHiPrio {
    Message,
    Menu,
    Cancelled,
}

impl FlowState for WarningHiPrio {
    #[inline]
    fn index(&'static self) -> usize {
        *self as usize
    }

    fn handle_swipe(&'static self, direction: SwipeDirection) -> StateChange {
        match (self, direction) {
            (Self::Message, SwipeDirection::Left) => Self::Menu.swipe(direction),
            (Self::Message, SwipeDirection::Up) => Self::Cancelled.swipe(direction),
            (Self::Menu, SwipeDirection::Right) => Self::Message.swipe(direction),
            _ => self.do_nothing(),
        }
    }

    fn handle_event(&'static self, msg: FlowMsg) -> StateChange {
        match (self, msg) {
            (Self::Message, FlowMsg::Info) => Self::Menu.transit(),
            (Self::Menu, FlowMsg::Choice(1)) => self.return_msg(FlowMsg::Confirmed),
            (Self::Menu, FlowMsg::Choice(_)) => Self::Cancelled.swipe_up(),
            (Self::Menu, FlowMsg::Cancelled) => Self::Message.swipe_right(),
            (Self::Cancelled, _) => self.return_msg(FlowMsg::Cancelled),
            _ => self.do_nothing(),
        }
    }
}

const EXTRA_PADDING: i16 = 6;

pub fn new_warning_hi_prio(
    title: TString<'static>,
    description: TString<'static>,
    value: TString<'static>,
) -> Result<SwipeFlow, error::Error> {
    let cancel: TString = TR::words__cancel_and_exit.into();
    let confirm: TString = "Continue anyway".into(); // FIXME: en.json has punctuation
    let done_title: TString = "Operation cancelled".into();

    // Message
    let paragraphs = [
        Paragraph::new(&theme::TEXT_MAIN_GREY_LIGHT, description),
        Paragraph::new(&theme::TEXT_MAIN_GREY_EXTRA_LIGHT, value).with_top_padding(EXTRA_PADDING),
    ]
    .into_paragraphs();
    let content_message = Frame::left_aligned(title, SwipeContent::new(paragraphs))
        .with_menu_button()
        .with_footer(TR::instructions__swipe_up.into(), Some(cancel))
        .with_danger()
        .with_swipe(SwipeDirection::Up, SwipeSettings::default())
        .with_swipe(SwipeDirection::Left, SwipeSettings::default())
        .map(|msg| matches!(msg, FrameMsg::Button(_)).then_some(FlowMsg::Info));
    // .one_button_request(ButtonRequestCode::Warning, br_name);

    // Menu
    let content_menu = Frame::left_aligned(
        "".into(),
        VerticalMenu::empty()
            .item(theme::ICON_CANCEL, "Cancel".into()) // TODO: button__cancel after it's lowercase
            .danger(theme::ICON_CHEVRON_RIGHT, confirm),
    )
    .with_cancel_button()
    .with_swipe(SwipeDirection::Right, SwipeSettings::immediate())
    .map(|msg| match msg {
        FrameMsg::Content(VerticalMenuChoiceMsg::Selected(i)) => Some(FlowMsg::Choice(i)),
        FrameMsg::Button(_) => Some(FlowMsg::Cancelled),
    });

    // Cancelled
    let content_cancelled = Frame::left_aligned(
        TR::words__title_done.into(),
        StatusScreen::new_neutral_timeout(done_title),
    )
    .with_footer(TR::instructions__continue_in_app.into(), None)
    .with_result_icon(theme::ICON_BULLET_CHECKMARK, theme::GREY_DARK)
    .map(|_| Some(FlowMsg::Cancelled));

    let res = SwipeFlow::new(&WarningHiPrio::Message)?
        .with_page(&WarningHiPrio::Message, content_message)?
        .with_page(&WarningHiPrio::Menu, content_menu)?
        .with_page(&WarningHiPrio::Cancelled, content_cancelled)?;
    Ok(res)
}
