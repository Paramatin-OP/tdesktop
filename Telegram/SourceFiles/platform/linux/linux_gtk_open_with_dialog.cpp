/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "platform/linux/linux_gtk_open_with_dialog.h"

#include "platform/linux/linux_gtk_integration_p.h"
#include "platform/linux/linux_gdk_helper.h"

#include <giomm.h>

namespace Platform {
namespace File {
namespace internal {
namespace {

using namespace Platform::Gtk;

rpl::event_stream<bool> GtkOpenWithDialogResponseStream;

struct GtkWidgetDeleter {
	void operator()(GtkWidget *widget) {
		gtk_widget_destroy(widget);
	}
};

bool Supported() {
	return (gtk_app_chooser_dialog_new != nullptr)
		&& (gtk_app_chooser_get_app_info != nullptr)
		&& (gtk_app_chooser_get_type != nullptr)
		&& (gtk_widget_get_window != nullptr)
		&& (gtk_widget_realize != nullptr)
		&& (gtk_widget_show != nullptr)
		&& (gtk_widget_destroy != nullptr);
}

class GtkOpenWithDialog {
public:
	GtkOpenWithDialog(
		const QString &parent,
		const QString &filepath);

private:
	static void handleResponse(GtkOpenWithDialog *dialog, int responseId);

	const Glib::RefPtr<Gio::File> _file;
	const std::unique_ptr<GtkWidget, GtkWidgetDeleter> _gtkWidget;
};

GtkOpenWithDialog::GtkOpenWithDialog(
		const QString &parent,
		const QString &filepath)
: _file(Gio::File::create_for_path(filepath.toStdString()))
, _gtkWidget(gtk_app_chooser_dialog_new(
		nullptr,
		GTK_DIALOG_MODAL,
		_file->gobj())) {
	g_signal_connect_swapped(
		_gtkWidget.get(),
		"response",
		G_CALLBACK(handleResponse),
		this);

	gtk_widget_realize(_gtkWidget.get());

	Platform::internal::GdkSetTransientFor(
		gtk_widget_get_window(_gtkWidget.get()),
		parent);

	gtk_widget_show(_gtkWidget.get());
}

void GtkOpenWithDialog::handleResponse(GtkOpenWithDialog *dialog, int responseId) {
	Glib::RefPtr<Gio::AppInfo> chosenAppInfo;
	bool result = true;

	switch (responseId) {
	case GTK_RESPONSE_OK:
		chosenAppInfo = Glib::wrap(gtk_app_chooser_get_app_info(
			GTK_APP_CHOOSER(dialog->_gtkWidget.get())));

		if (chosenAppInfo) {
			try {
				chosenAppInfo->launch_uri(dialog->_file->get_uri());
			} catch (...) {
			}
			chosenAppInfo = {};
		}

		break;

	case GTK_RESPONSE_CANCEL:
	case GTK_RESPONSE_DELETE_EVENT:
		break;

	default:
		result = false;
		break;
	}

	GtkOpenWithDialogResponseStream.fire_copy(result);
	delete dialog;
}

} // namespace

bool ShowGtkOpenWithDialog(
		const QString &parent,
		const QString &filepath) {
	if (!Supported()) {
		return false;
	}

	return new GtkOpenWithDialog(parent, filepath);
}

rpl::producer<bool> GtkOpenWithDialogResponse() {
	return GtkOpenWithDialogResponseStream.events();
}

} // namespace internal
} // namespace File
} // namespace Platform
