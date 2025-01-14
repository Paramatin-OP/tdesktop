/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

namespace Platform {
namespace File {
namespace internal {

bool ShowGtkOpenWithDialog(
	const QString &parent,
	const QString &filepath);

[[nodiscard]] rpl::producer<bool> GtkOpenWithDialogResponse();

} // namespace internal
} // namespace File
} // namespace Platform
