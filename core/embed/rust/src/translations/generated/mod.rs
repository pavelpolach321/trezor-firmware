#[cfg(feature = "model_mercury")]
mod translated_string;
#[cfg(feature = "model_mercury")]
pub use translated_string::TranslatedString;

#[cfg(any(feature = "model_tr", feature = "model_tt"))]
mod translated_string_upper;
#[cfg(any(feature = "model_tr", feature = "model_tt"))]
pub use translated_string_upper::TranslatedString;
