//! generated from ${THIS_FILE.name}
//! (by running `make templates` in `core`)
//! do not edit manually!

#![cfg_attr(rustfmt, rustfmt_skip)]
<%
import json
from trezorlib._internal.translations import uppercase_titles_and_buttons

ALTCOIN_PREFIXES = (
    "binance",
    "cardano",
    "eos",
    "ethereum",
    "fido",
    "monero",
    "nem",
    "ripple",
    "solana",
    "stellar",
    "tezos",
    "u2f",
)

TR_DIR = ROOT / "core" / "translations"

order_file = TR_DIR / "order.json"
order_index_name = json.loads(order_file.read_text())
order = {int(k): v for k, v in order_index_name.items()}


en_file = TR_DIR / "en.json"
en_data = json.loads(en_file.read_text())["translations"]

%>\
#[cfg(feature = "micropython")]
use crate::micropython::qstr::Qstr;

#[derive(Debug, Copy, Clone, FromPrimitive, PartialEq, Eq)]
#[repr(u16)]
#[allow(non_camel_case_types)]
pub enum TranslatedString {
% for idx, name in order.items():
    %if any(name.startswith(prefix + "__") for prefix in ALTCOIN_PREFIXES):
    #[cfg(feature = "universal_fw")]
    %endif
    ${name} = ${idx},  // ${json.dumps(en_data.get(name, '""'))}
% endfor
}

impl TranslatedString {
    pub fn untranslated(self) -> &'static str {
        match self {
% for name in order.values():
            %if any(name.startswith(prefix + "__") for prefix in ALTCOIN_PREFIXES):
            #[cfg(feature = "universal_fw")]
            %endif
            Self::${name} => ${uppercase_titles_and_buttons(json.dumps(en_data.get(name, '""')), name, "T2T1")},
% endfor
        }
    }

    #[cfg(feature = "micropython")]
    pub fn from_qstr(qstr: Qstr) -> Option<Self> {
        match qstr {
% for name in order.values():
            %if any(name.startswith(prefix + "__") for prefix in ALTCOIN_PREFIXES):
            #[cfg(feature = "universal_fw")]
            %endif
            Qstr::MP_QSTR_${name} => Some(Self::${name}),
% endfor
            _ => None,
        }
    }
}
