from typing import TYPE_CHECKING

import trezorui2
from trezor import TR, io, loop, ui, utils
from trezor.enums import ButtonRequestType
from trezor.messages import ButtonAck, ButtonRequest
from trezor.wire import ActionCancelled, context

from ..common import raise_if_not_confirmed, interact, with_info, draw_simple

if TYPE_CHECKING:
    from typing import Any, Awaitable, Iterable, NoReturn, Sequence, TypeVar

    from ..common import ExceptionType, PropertyType

    T = TypeVar("T")


BR_CODE_OTHER = ButtonRequestType.Other  # global_import_cache

CONFIRMED = trezorui2.CONFIRMED
CANCELLED = trezorui2.CANCELLED
INFO = trezorui2.INFO


def confirm_action(
    br_name: str,
    title: str,
    action: str | None = None,
    description: str | None = None,
    description_param: str | None = None,
    subtitle: str | None = None,
    verb: str | None = None,
    verb_cancel: str | None = None,
    hold: bool = False,
    hold_danger: bool = False,
    reverse: bool = False,
    exc: ExceptionType = ActionCancelled,
    br_code: ButtonRequestType = BR_CODE_OTHER,
    prompt_screen: bool = False,
    prompt_title: str | None = None,
) -> Awaitable[None]:
    if description is not None and description_param is not None:
        description = description.format(description_param)

    return raise_if_not_confirmed(
        trezorui2.confirm_action(
            title=title,
            action=action,
            description=description,
            subtitle=subtitle,
            verb=verb,
            verb_cancel=verb_cancel,
            hold=hold,
            hold_danger=hold_danger,
            reverse=reverse,
            prompt_screen=prompt_screen,
            prompt_title=prompt_title or title,
        ),
        br_name,
        br_code,
        exc,
    )


def confirm_single(
    br_name: str,
    title: str,
    description: str,
    description_param: str | None = None,
    verb: str | None = None,
) -> Awaitable[None]:
    description_param = description_param or ""

    # Placeholders are coming from translations in form of {0}
    template_str = "{0}"
    if template_str not in description:
        template_str = "{}"

    begin, _separator, end = description.partition(template_str)
    return raise_if_not_confirmed(
        trezorui2.confirm_emphasized(
            title=title,
            items=(begin, (True, description_param), end),
            verb=verb,
        ),
        br_name,
        ButtonRequestType.ProtectCall,
    )


def confirm_reset_device(_title: str, recovery: bool = False) -> Awaitable[None]:
    if recovery:
        return raise_if_not_confirmed(trezorui2.flow_confirm_reset_recover(), None)
    else:
        return raise_if_not_confirmed(trezorui2.flow_confirm_reset_create(), None)


async def show_wallet_created_success() -> None:
    await interact(
        trezorui2.show_success(title=TR.backup__new_wallet_created, description=""),
        "backup_device",
        ButtonRequestType.ResetDevice,
    )


async def prompt_backup() -> bool:
    result = await interact(
        trezorui2.flow_prompt_backup(),
        "backup_device",
        ButtonRequestType.ResetDevice,
    )
    return result is CONFIRMED


def confirm_path_warning(
    path: str,
    path_type: str | None = None,
) -> Awaitable[None]:
    description = (
        TR.addr_mismatch__wrong_derivation_path
        if not path_type
        else f"{TR.words__unknown} {path_type.lower()}."
    )
    return raise_if_not_confirmed(
        trezorui2.flow_warning_hi_prio(
            title=f"{TR.words__warning}!", description=description, value=path
        ),
        "path_warning",
        br_code=ButtonRequestType.UnknownDerivationPath,
    )


def confirm_multisig_warning() -> Awaitable[None]:
    return raise_if_not_confirmed(
        trezorui2.flow_warning_hi_prio(
            title=f"{TR.words__important}!",
            description=TR.send__receiving_to_multisig,
        ),
        "warning_multisig",
        br_code=ButtonRequestType.Warning,
    )


def confirm_homescreen(
    image: bytes,
) -> Awaitable[None]:

    from trezor import workflow

    workflow.close_others()

    return raise_if_not_confirmed(
        trezorui2.confirm_homescreen(
            title=TR.homescreen__title_set,
            image=image,
        ),
        "set_homesreen",
        ButtonRequestType.ProtectCall,
    )


async def show_address(
    address: str,
    *,
    title: str | None = None,
    address_qr: str | None = None,
    case_sensitive: bool = True,
    path: str | None = None,
    account: str | None = None,
    network: str | None = None,
    multisig_index: int | None = None,
    xpubs: Sequence[str] = (),
    mismatch_title: str | None = None,
    details_title: str | None = None,
    br_name: str = "show_address",
    br_code: ButtonRequestType = ButtonRequestType.Address,
    chunkify: bool = False,
) -> None:
    def xpub_title(i: int) -> str:
        result = f"Multisig XPUB #{i + 1}\n"
        result += (
            f"({TR.address__title_yours.lower()})"
            if i == multisig_index
            else f"({TR.address__title_cosigner.lower()})"
        )
        return result

    await raise_if_not_confirmed(
        trezorui2.flow_get_address(
            address=address,
            title=title or TR.address__title_receive_address,
            description=network or "",
            extra=None,
            chunkify=chunkify,
            address_qr=address if address_qr is None else address_qr,
            case_sensitive=case_sensitive,
            account=account,
            path=path,
            xpubs=[(xpub_title(i), xpub) for i, xpub in enumerate(xpubs)],
            br_name=br_name,
            br_code=br_code,
        ),
        None,
    )


def show_pubkey(
    pubkey: str,
    title: str | None = None,
    *,
    account: str | None = None,
    path: str | None = None,
    mismatch_title: str | None = None,
    br_name: str = "show_pubkey",
) -> Awaitable[None]:
    title = title or TR.address__public_key  # def_arg
    mismatch_title = mismatch_title or TR.addr_mismatch__key_mismatch  # def_arg
    return show_address(
        address=pubkey,
        title=title,
        account=account,
        path=path,
        br_name=br_name,
        br_code=ButtonRequestType.PublicKey,
        mismatch_title=mismatch_title,
        chunkify=False,
    )


async def show_error_and_raise(
    br_name: str,
    content: str,
    subheader: str | None = None,
    button: str | None = None,
    exc: ExceptionType = ActionCancelled,
) -> NoReturn:
    button = button or TR.buttons__try_again  # def_arg
    await interact(
        trezorui2.show_error(
            title=subheader or "",
            description=content,
            button=button,
            allow_cancel=False,
        ),
        br_name,
        BR_CODE_OTHER,
    )
    raise exc


def show_warning(
    br_name: str,
    content: str,
    subheader: str | None = None,
    button: str | None = None,
    br_code: ButtonRequestType = ButtonRequestType.Warning,
) -> Awaitable[None]:
    button = button or TR.buttons__continue  # def_arg
    return raise_if_not_confirmed(
        trezorui2.show_warning(
            title=TR.words__important,
            value=content,
            button=subheader or TR.words__continue_anyway,
        ),
        br_name,
        br_code,
    )


def show_success(
    br_name: str,
    content: str,
    subheader: str | None = None,
    button: str | None = None,
) -> Awaitable[None]:
    return raise_if_not_confirmed(
        trezorui2.show_success(
            title=content,
            description=subheader if subheader else "",
        ),
        br_name,
        ButtonRequestType.Success,
    )


async def confirm_output(
    address: str,
    amount: str,
    title: str | None = None,
    hold: bool = False,
    br_code: ButtonRequestType = ButtonRequestType.ConfirmOutput,
    address_label: str | None = None,
    output_index: int | None = None,
    chunkify: bool = False,
    source_account: str | None = None,
    source_account_path: str | None = None,
) -> None:
    if address_label is not None:
        title = address_label
    elif title is not None:
        pass
    elif output_index is not None:
        title = f"{TR.words__recipient} #{output_index + 1}"
    else:
        title = TR.send__title_sending_to

    await raise_if_not_confirmed(
        trezorui2.flow_confirm_output(
            address=address,
            amount=amount,
            title=title,
            chunkify=chunkify,
            account=source_account,
            account_path=source_account_path,
            br_code=br_code,
            br_name="confirm_output",
        ),
        None,
    )


async def should_show_payment_request_details(
    recipient_name: str,
    amount: str,
    memos: list[str],
) -> bool:
    """Return True if the user wants to show payment request details (they click a
    special button) and False when the user wants to continue without showing details.

    Raises ActionCancelled if the user cancels.
    """
    result = await interact(
        trezorui2.confirm_with_info(
            title=TR.send__title_sending,
            items=[(ui.NORMAL, f"{amount} to\n{recipient_name}")]
            + [(ui.NORMAL, memo) for memo in memos],
            button=TR.buttons__confirm,
            info_button=TR.buttons__details,
        ),
        "confirm_payment_request",
        ButtonRequestType.ConfirmOutput,
    )

    if result is CONFIRMED:
        return False
    elif result is INFO:
        return True
    else:
        raise ActionCancelled


async def should_show_more(
    title: str,
    para: Iterable[tuple[int, str | bytes]],
    button_text: str | None = None,
    br_name: str = "should_show_more",
    br_code: ButtonRequestType = BR_CODE_OTHER,
    confirm: str | bytes | None = None,
) -> bool:
    """Return True if the user wants to show more (they click a special button)
    and False when the user wants to continue without showing details.

    Raises ActionCancelled if the user cancels.
    """
    button_text = button_text or TR.buttons__show_all  # def_arg
    if confirm is None or not isinstance(confirm, str):
        confirm = TR.buttons__confirm

    result = await interact(
        trezorui2.confirm_with_info(
            title=title,
            items=para,
            button=confirm,
            info_button=button_text,
        ),
        br_name,
        br_code,
    )

    if result is CONFIRMED:
        return False
    elif result is INFO:
        return True
    else:
        assert result is CANCELLED
        raise ActionCancelled


async def _confirm_ask_pagination(
    br_name: str,
    title: str,
    data: bytes | str,
    description: str,
    br_code: ButtonRequestType,
) -> None:
    # TODO: make should_show_more/confirm_more accept bytes directly
    if isinstance(data, bytes):
        from ubinascii import hexlify

        data = hexlify(data).decode()
    while True:
        if not await should_show_more(
            title,
            para=[(ui.NORMAL, description), (ui.MONO, data)],
            br_name=br_name,
            br_code=br_code,
        ):
            return

        result = await interact(
            trezorui2.confirm_more(
                title=title,
                button=TR.buttons__close,
                items=[(ui.MONO, data)],
            ),
            br_name,
            br_code,
        )
        assert result in (CONFIRMED, CANCELLED)

    assert False


def confirm_blob(
    br_name: str,
    title: str,
    data: bytes | str,
    description: str | None = None,
    verb: str | None = None,
    verb_cancel: str | None = None,
    hold: bool = False,
    br_code: ButtonRequestType = BR_CODE_OTHER,
    ask_pagination: bool = False,
    chunkify: bool = False,
    prompt_screen: bool = True,
) -> Awaitable[None]:
    layout = trezorui2.confirm_blob(
        title=title,
        description=description,
        data=data,
        extra=None,
        hold=hold,
        verb=verb,
        verb_cancel=verb_cancel,
        chunkify=chunkify,
        prompt_screen=prompt_screen,
    )

    if ask_pagination and layout.page_count() > 1:
        assert not hold
        return _confirm_ask_pagination(br_name, title, data, description or "", br_code)

    else:
        return raise_if_not_confirmed(
            layout,
            br_name,
            br_code,
        )


def confirm_address(
    title: str,
    address: str,
    description: str | None = None,
    br_name: str = "confirm_address",
    br_code: ButtonRequestType = BR_CODE_OTHER,
) -> Awaitable[None]:
    return confirm_value(
        title,
        address,
        description or "",
        br_name,
        br_code,
        verb=TR.buttons__confirm,
    )


def confirm_text(
    br_name: str,
    title: str,
    data: str,
    description: str | None = None,
    br_code: ButtonRequestType = BR_CODE_OTHER,
) -> Awaitable[None]:
    return confirm_value(
        title,
        data,
        description or "",
        br_name,
        br_code,
        verb=TR.buttons__confirm,
    )


def confirm_amount(
    title: str,
    amount: str,
    description: str | None = None,
    br_name: str = "confirm_amount",
    br_code: ButtonRequestType = BR_CODE_OTHER,
) -> Awaitable[None]:
    description = description or f"{TR.words__amount}:"  # def_arg
    return confirm_value(
        title,
        amount,
        description,
        br_name,
        br_code,
        verb=TR.buttons__confirm,
    )


def confirm_value(
    title: str,
    value: str,
    description: str,
    br_name: str,
    br_code: ButtonRequestType = BR_CODE_OTHER,
    *,
    verb: str | None = None,
    subtitle: str | None = None,
    hold: bool = False,
    value_text_mono: bool = True,
    info_items: Iterable[tuple[str, str]] | None = None,
    info_title: str | None = None,
    chunkify_info: bool = False,
) -> Awaitable[None]:
    """General confirmation dialog, used by many other confirm_* functions."""

    if not verb and not hold:
        raise ValueError("Either verb or hold=True must be set")

    info_items = info_items or []
    info_layout = trezorui2.show_info_with_cancel(
        title=info_title if info_title else TR.words__title_information,
        items=info_items,
        chunkify=chunkify_info,
    )

    return with_info(
        trezorui2.confirm_value(
            title=title,
            subtitle=subtitle,
            description=description,
            value=value,
            verb=verb,
            hold=hold,
            info_button=bool(info_items),
            text_mono=value_text_mono,
        ),
        info_layout,
        br_name,
        br_code,
    )


def confirm_properties(
    br_name: str,
    title: str,
    props: Iterable[PropertyType],
    hold: bool = False,
    br_code: ButtonRequestType = ButtonRequestType.ConfirmOutput,
) -> Awaitable[None]:
    # Monospace flag for values that are bytes.
    items = [(prop[0], prop[1], isinstance(prop[1], bytes)) for prop in props]

    return raise_if_not_confirmed(
        trezorui2.confirm_properties(
            title=title,
            items=items,
            hold=hold,
        ),
        br_name,
        br_code,
    )


def confirm_total(
    total_amount: str,
    fee_amount: str,
    title: str | None = None,
    total_label: str | None = None,
    fee_label: str | None = None,
    source_account: str | None = None,
    source_account_path: str | None = None,
    fee_rate_amount: str | None = None,
    br_name: str = "confirm_total",
    br_code: ButtonRequestType = ButtonRequestType.SignTx,
) -> Awaitable[None]:
    title = title or TR.words__title_summary  # def_arg
    total_label = total_label or TR.send__total_amount  # def_arg
    fee_label = fee_label or TR.send__incl_transaction_fee  # def_arg

    items = [
        (total_label, total_amount),
        (fee_label, fee_amount),
    ]
    fee_items = []
    account_items = []
    if source_account:
        account_items.append((TR.confirm_total__sending_from_account, source_account))
    if source_account_path:
        account_items.append((TR.address_details__derivation_path, source_account_path))
    if fee_rate_amount:
        fee_items.append((TR.confirm_total__fee_rate, fee_rate_amount))

    return raise_if_not_confirmed(
        trezorui2.flow_confirm_summary(
            title=title,
            items=items,
            fee_items=fee_items,
            account_items=account_items,
            br_name=br_name,
            br_code=br_code,
        ),
        None,
    )


def confirm_summary(
    items: Iterable[tuple[str, str]],
    title: str | None = None,
    info_items: Iterable[tuple[str, str]] | None = None,
    info_title: str | None = None,
    br_name: str = "confirm_total",
    br_code: ButtonRequestType = ButtonRequestType.SignTx,
) -> Awaitable[None]:
    # TODO: info_title
    title = title or TR.words__title_summary  # def_arg

    return raise_if_not_confirmed(
        trezorui2.flow_confirm_summary(
            title=title,
            items=items or (),
            fee_items=(),
            account_items=info_items or (),
            br_name=br_name,
            br_code=br_code,
        ),
        None,
    )


if not utils.BITCOIN_ONLY:

    async def confirm_ethereum_tx(
        recipient: str,
        total_amount: str,
        maximum_fee: str,
        items: Iterable[tuple[str, str]],
        br_name: str = "confirm_ethereum_tx",
        br_code: ButtonRequestType = ButtonRequestType.SignTx,
        chunkify: bool = False,
    ) -> None:
        total_layout = trezorui2.confirm_total(
            title=TR.words__title_summary,
            items=[
                (f"{TR.words__amount}:", total_amount),
                (TR.send__maximum_fee, maximum_fee),
            ],
            info_button=True,
            cancel_arrow=True,
        )
        info_layout = trezorui2.show_info_with_cancel(
            title=TR.confirm_total__title_fee,
            items=items,
        )

        while True:
            # Allowing going back and forth between recipient and summary/details
            await confirm_blob(
                br_name,
                TR.words__recipient,
                recipient,
                verb=TR.buttons__continue,
                chunkify=chunkify,
                prompt_screen=False,
            )

            try:
                await with_info(total_layout, info_layout, br_name, br_code)
                break
            except ActionCancelled:
                continue

    async def confirm_ethereum_staking_tx(
        title: str,
        intro_question: str,
        verb: str,
        total_amount: str,
        maximum_fee: str,
        address: str,
        address_title: str,
        info_items: Iterable[tuple[str, str]],
        chunkify: bool = False,
        br_name: str = "confirm_ethereum_staking_tx",
        br_code: ButtonRequestType = ButtonRequestType.SignTx,
    ) -> None:

        # intro
        await confirm_value(
            title,
            intro_question,
            "",
            br_name,
            br_code,
            verb=verb,
            value_text_mono=False,
            info_items=(("", address),),
            info_title=address_title,
            chunkify_info=chunkify,
        )

        # confirmation
        if verb == TR.ethereum__staking_claim:
            items = ((TR.send__maximum_fee, maximum_fee),)
        else:
            items = (
                (TR.words__amount + ":", total_amount),
                (TR.send__maximum_fee, maximum_fee),
            )
        await confirm_summary(
            items,  # items
            title=title,
            info_title=TR.confirm_total__title_fee,
            info_items=info_items,
            br_name=br_name,
            br_code=br_code,
        )

    def confirm_solana_tx(
        amount: str,
        fee: str,
        items: Iterable[tuple[str, str]],
        amount_title: str | None = None,
        fee_title: str | None = None,
        br_name: str = "confirm_solana_tx",
        br_code: ButtonRequestType = ButtonRequestType.SignTx,
    ) -> Awaitable[None]:
        amount_title = (
            amount_title if amount_title is not None else f"{TR.words__amount}:"
        )  # def_arg
        fee_title = fee_title or TR.words__fee  # def_arg
        return confirm_summary(
            ((amount_title, amount), (fee_title, fee)),
            info_items=items,
            br_name=br_name,
            br_code=br_code,
        )


def confirm_joint_total(spending_amount: str, total_amount: str) -> Awaitable[None]:
    return confirm_summary(
        items=[
            (TR.send__you_are_contributing, spending_amount),
            (TR.send__to_the_total_amount, total_amount),
        ],
        title=TR.send__title_joint_transaction,
        br_name="confirm_joint_total",
        br_code=ButtonRequestType.SignTx,
    )


def confirm_metadata(
    br_name: str,
    title: str,
    content: str,
    param: str | None = None,
    br_code: ButtonRequestType = ButtonRequestType.SignTx,
    hold: bool = False,
    verb: str | None = None,
) -> Awaitable[None]:
    verb = verb or TR.buttons__continue  # def_arg
    return confirm_action(
        br_name,
        title=title,
        action="",
        description=content,
        description_param=param,
        verb=verb,
        hold=hold,
        br_code=br_code,
    )


def confirm_replacement(title: str, txid: str) -> Awaitable[None]:
    return confirm_blob(
        "confirm_replacement",
        title,
        txid,
        TR.send__transaction_id,
        TR.buttons__continue,
        br_code=ButtonRequestType.SignTx,
    )


async def confirm_modify_output(
    address: str,
    sign: int,
    amount_change: str,
    amount_new: str,
) -> None:
    address_layout = trezorui2.confirm_blob(
        title=TR.modify_amount__title,
        data=address,
        verb=TR.buttons__continue,
        verb_cancel=None,
        description=f"{TR.words__address}:",
        extra=None,
    )
    modify_layout = trezorui2.confirm_modify_output(
        sign=sign,
        amount_change=amount_change,
        amount_new=amount_new,
    )

    send_button_request = True
    while True:
        await raise_if_not_confirmed(
            address_layout,
            "modify_output" if send_button_request else None,
            ButtonRequestType.ConfirmOutput,
        )
        result = await interact(
            modify_layout,
            "modify_output" if send_button_request else None,
            ButtonRequestType.ConfirmOutput,
            raise_on_cancel=None,
        )
        send_button_request = False

        if result is CONFIRMED:
            break


def confirm_modify_fee(
    title: str,
    sign: int,
    user_fee_change: str,
    total_fee_new: str,
    fee_rate_amount: str | None = None,
) -> Awaitable[None]:
    fee_layout = trezorui2.confirm_modify_fee(
        title=title,
        sign=sign,
        user_fee_change=user_fee_change,
        total_fee_new=total_fee_new,
        fee_rate_amount=fee_rate_amount,
    )
    items: list[tuple[str, str]] = []
    if fee_rate_amount:
        items.append((TR.bitcoin__new_fee_rate, fee_rate_amount))
    info_layout = trezorui2.show_info_with_cancel(
        title=TR.confirm_total__title_fee,
        items=items,
    )
    return with_info(fee_layout, info_layout, "modify_fee", ButtonRequestType.SignTx)


def confirm_coinjoin(max_rounds: int, max_fee_per_vbyte: str) -> Awaitable[None]:
    return raise_if_not_confirmed(
        trezorui2.confirm_coinjoin(
            max_rounds=str(max_rounds),
            max_feerate=max_fee_per_vbyte,
        ),
        "coinjoin_final",
        BR_CODE_OTHER,
    )


# TODO cleanup @ redesign
def confirm_sign_identity(
    proto: str, identity: str, challenge_visual: str | None
) -> Awaitable[None]:
    return confirm_blob(
        "sign_identity",
        f"{TR.words__sign} {proto}",
        identity,
        challenge_visual + "\n" if challenge_visual else "",
        br_code=BR_CODE_OTHER,
    )


async def confirm_signverify(
    message: str,
    address: str,
    verify: bool,
    path: str | None = None,
    account: str | None = None,
    chunkify: bool = False,
) -> None:
    if verify:
        address_title = TR.sign_message__verify_address
        br_name = "verify_message"
    else:
        address_title = TR.sign_message__confirm_address
        br_name = "sign_message"

    address_layout = trezorui2.confirm_address(
        title=address_title,
        data=address,
        description="",
        verb=TR.buttons__continue,
        extra=None,
        chunkify=chunkify,
    )

    items: list[tuple[str, str]] = []
    if account is not None:
        items.append((TR.words__account, account))
    if path is not None:
        items.append((TR.address_details__derivation_path, path))
    items.append(
        (
            TR.sign_message__message_size,
            TR.sign_message__bytes_template.format(len(message)),
        )
    )

    info_layout = trezorui2.show_info_with_cancel(
        title=TR.words__title_information,
        items=items,
        horizontal=True,
    )

    message_layout = trezorui2.confirm_blob(
        title=TR.sign_message__confirm_message,
        description=None,
        data=message,
        extra=None,
        hold=not verify,
        verb=TR.buttons__confirm if verify else None,
    )

    while True:
        result = await with_info(
            address_layout, info_layout, br_name, br_code=BR_CODE_OTHER
        )
        if result is not CONFIRMED:
            result = await interact(
                trezorui2.show_mismatch(title=TR.addr_mismatch__mismatch), None
            )
            assert result in (CONFIRMED, CANCELLED)
            # Right button aborts action, left goes back to showing address.
            if result is CONFIRMED:
                raise ActionCancelled
            continue

        result = await interact(message_layout, br_name, BR_CODE_OTHER)
        if result is CONFIRMED:
            break


def error_popup(
    title: str,
    description: str,
    subtitle: str | None = None,
    description_param: str = "",
    *,
    button: str = "",
    timeout_ms: int = 0,
) -> ui.LayoutObj[ui.UiResult]:
    if not button and not timeout_ms:
        raise ValueError("Either button or timeout_ms must be set")

    if subtitle:
        title += f"\n{subtitle}"
    return trezorui2.show_error(
        title=title,
        description=description.format(description_param),
        button=button,
        time_ms=timeout_ms,
        allow_cancel=False,
    )


def request_passphrase_on_host() -> None:
    draw_simple(
        trezorui2.show_simple(
            title=None,
            description=TR.passphrase__please_enter,
        )
    )


def show_wait_text(message: str) -> None:
    draw_simple(trezorui2.show_wait_text(message))


def request_passphrase_on_device(max_len: int) -> Awaitable[str]:
    result = interact(
        trezorui2.request_passphrase(
            prompt=TR.passphrase__title_enter, max_len=max_len
        ),
        "passphrase_device",
        ButtonRequestType.PassphraseEntry,
        raise_on_cancel=ActionCancelled("Passphrase entry cancelled"),
    )
    return result  # type: ignore ['UiResult' is incompatible with 'str']


def request_pin_on_device(
    prompt: str,
    attempts_remaining: int | None,
    allow_cancel: bool,
    wrong_pin: bool = False,
) -> Awaitable[str]:
    from trezor.wire import PinCancelled

    if attempts_remaining is None:
        subprompt = ""
    elif attempts_remaining == 1:
        subprompt = TR.pin__last_attempt
    else:
        subprompt = f"{attempts_remaining} {TR.pin__tries_left}"

    result = interact(
        trezorui2.request_pin(
            prompt=prompt,
            subprompt=subprompt,
            allow_cancel=allow_cancel,
            wrong_pin=wrong_pin,
        ),
        "pin_device",
        ButtonRequestType.PinEntry,
        raise_on_cancel=PinCancelled,
    )
    return result  # type: ignore ['UiResult' is incompatible with 'str']


async def confirm_reenter_pin(is_wipe_code: bool = False) -> None:
    """Not supported for Mercury."""
    pass


def pin_mismatch_popup(is_wipe_code: bool = False) -> Awaitable[ui.UiResult]:
    title = TR.wipe_code__mismatch if is_wipe_code else TR.pin__mismatch
    description = TR.wipe_code__enter_new if is_wipe_code else TR.pin__reenter_new

    return interact(
        error_popup(
            title,
            description,
            button=TR.buttons__try_again,
        ),
        "pin_mismatch",
        BR_CODE_OTHER,
    )


async def wipe_code_same_as_pin_popup() -> Awaitable[ui.UiResult]:
    return interact(
        error_popup(
            TR.wipe_code__invalid,
            TR.wipe_code__diff_from_pin,
            button=TR.buttons__try_again,
        ),
        "wipe_code_same_as_pin",
        BR_CODE_OTHER,
    )


def confirm_set_new_pin(
    br_name: str,
    title: str,
    description: str,
    information: str,
    br_code: ButtonRequestType = BR_CODE_OTHER,
) -> Awaitable[None]:
    return raise_if_not_confirmed(
        trezorui2.flow_confirm_set_new_pin(title=title, description=description),
        br_name,
        br_code,
    )


def confirm_firmware_update(description: str, fingerprint: str) -> Awaitable[None]:
    return raise_if_not_confirmed(
        trezorui2.confirm_firmware_update(
            description=description, fingerprint=fingerprint
        ),
        "firmware_update",
        BR_CODE_OTHER,
    )


def set_brightness(current: int | None = None) -> Awaitable[None]:
    return raise_if_not_confirmed(
        trezorui2.set_brightness(current=current),
        "set_brightness",
        BR_CODE_OTHER,
    )


def tutorial(br_code: ButtonRequestType = BR_CODE_OTHER) -> Awaitable[None]:
    """Showing users how to interact with the device."""
    return raise_if_not_confirmed(
        trezorui2.tutorial(),
        "tutorial",
        br_code,
    )
