#![allow(dead_code)]
// These are bad style at best and can conceal problems.
#![allow(unused_imports)]
#![allow(unused_mut)]
#![allow(unused_variables)]
#![allow(clippy::assign_op_pattern)]
#![allow(clippy::char_lit_as_u8)]
#![allow(clippy::field_reassign_with_default)]
#![allow(clippy::identity_op)]
#![allow(clippy::if_same_then_else)]
#![allow(clippy::len_zero)]
#![allow(clippy::manual_map)]
#![allow(clippy::neg_cmp_op_on_partial_ord)]
#![allow(clippy::nonminimal_bool)]

mod error;
mod ffi;
mod tor_crypto;
mod object_registry;
mod tor_controller;
mod honk_rpc;
mod gosling;
#[cfg(test)]
mod test_utils;
