use crate::{
    error,
    strutil::ShortString,
    translations::TR,
    ui::{
        component::{ComponentExt, SwipeDirection},
        flow::{
            base::{DecisionBuilder as _, StateChange},
            FlowMsg, FlowState, SwipeFlow,
        },
    },
};

use super::super::component::{
    Frame, FrameMsg, PassphraseKeyboard, PassphraseKeyboardMsg, PromptMsg, PromptScreen,
};

#[derive(Copy, Clone, PartialEq, Eq)]
pub enum RequestPassphrase {
    Keypad,
    ConfirmEmpty,
}

impl FlowState for RequestPassphrase {
    #[inline]
    fn index(&'static self) -> usize {
        *self as usize
    }

    fn handle_swipe(&'static self, _direction: SwipeDirection) -> StateChange {
        self.do_nothing()
    }

    fn handle_event(&'static self, msg: FlowMsg) -> StateChange {
        match (self, msg) {
            (Self::Keypad, FlowMsg::Text(s)) => {
                if s.is_empty() {
                    Self::ConfirmEmpty.transit()
                } else {
                    self.return_msg(FlowMsg::Text(s))
                }
            }
            (Self::Keypad, FlowMsg::Cancelled) => self.return_msg(FlowMsg::Cancelled),
            (Self::ConfirmEmpty, FlowMsg::Cancelled) => Self::Keypad.transit(),
            (Self::ConfirmEmpty, FlowMsg::Confirmed) => {
                self.return_msg(FlowMsg::Text(ShortString::new()))
            }
            _ => self.do_nothing(),
        }
    }
}

pub fn new_request_passphrase() -> Result<SwipeFlow, error::Error> {
    let content_confirm_empty = Frame::left_aligned(
        TR::passphrase__continue_with_empty_passphrase.into(),
        PromptScreen::new_yes_or_no(),
    )
    .map(|msg| match msg {
        FrameMsg::Content(PromptMsg::Confirmed) => Some(FlowMsg::Confirmed),
        FrameMsg::Content(PromptMsg::Cancelled) => Some(FlowMsg::Cancelled),
        _ => None,
    });

    let content_keypad = PassphraseKeyboard::new().map(|msg| match msg {
        PassphraseKeyboardMsg::Confirmed(s) => Some(FlowMsg::Text(s)),
        PassphraseKeyboardMsg::Cancelled => Some(FlowMsg::Cancelled),
    });

    let res = SwipeFlow::new(&RequestPassphrase::Keypad)?
        .with_page(&RequestPassphrase::Keypad, content_keypad)?
        .with_page(&RequestPassphrase::ConfirmEmpty, content_confirm_empty)?;
    Ok(res)
}
