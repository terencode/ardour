/*
    Copyright (C) 2000 Paul Davis 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

/* this file exists solely to break compilation dependencies that
   would connect changes to the mixer or editor objects.
*/

#include <cstdio>

#include <gtkmm/accelmap.h>

#include <pbd/error.h>

#include "ardour_ui.h"
#include "public_editor.h"
#include "mixer_ui.h"
#include "keyboard.h"
#include "splash.h"
#include "route_params_ui.h"
#include "i18n.h"

using namespace sigc;
using namespace Gtk;
using namespace PBD;

namespace ARDOUR {
	class Session;
	class Route;
}

void
ARDOUR_UI::shutdown ()
{
	if (session) {
		/* we're exiting cleanly, so remove any auto-save data */
		session->remove_pending_capture_state ();
		session = 0;
	}
	ui_config->save_state();
}

void
ARDOUR_UI::we_have_dependents ()
{
	setup_keybindings ();
	editor->UpdateAllTransportClocks.connect (mem_fun (*this, &ARDOUR_UI::update_transport_clocks));
}

static void 
accel_map_changed (GtkAccelMap* map,
		   gchar* path,
		   guint  key,
		   GdkModifierType mod,
		   gpointer arg)
{
	static_cast<ARDOUR_UI*>(arg)->save_keybindings ();
}

void
ARDOUR_UI::setup_keybindings ()
{
	install_actions ();
	RedirectBox::register_actions ();
	
	cerr << "loading bindings from " << keybindings_path << endl;

	try {
		AccelMap::load (keybindings_path);
	} catch (...) {
		error << string_compose (_("Ardour key bindings file not found at \"%1\" or contains errors."), keybindings_path)
		      << endmsg;
	}

	/* catch changes */

	GtkAccelMap* accelmap = gtk_accel_map_get();
	g_signal_connect (accelmap, "changed", (GCallback) accel_map_changed, this);

	

}

void
ARDOUR_UI::connect_dependents_to_session (ARDOUR::Session *s)
{
	editor->connect_to_session (s);
	mixer->connect_to_session (s);

	/* its safe to do this now */
	
	s->restore_history ("");
}

static bool
_hide_splash (gpointer arg)
{
	((ARDOUR_UI*)arg)->hide_splash();
	return false;
}

void
ARDOUR_UI::goto_editor_window ()
{
	if (splash && splash->is_visible()) {
		// in 2 seconds, hide the splash screen 
		Glib::signal_timeout().connect (bind (sigc::ptr_fun (_hide_splash), this), 2000);
	}

	editor->show_window ();
	editor->present();
	flush_pending ();
}

void
ARDOUR_UI::goto_mixer_window ()
{
	mixer->show_window ();
	mixer->present();
	flush_pending ();
}

gint
ARDOUR_UI::exit_on_main_window_close (GdkEventAny *ev)
{
	finish();
	return TRUE;
}
