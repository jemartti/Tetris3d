#include "appwindow.hpp"

#include <iostream>

AppWindow::AppWindow()
{
	set_title( "488 Tetrominoes on the Wall" );

	// Utilities for constructing things that go into menus, which
	// we'll set up next.
	using Gtk::Menu_Helpers::MenuElem;
	using Gtk::Menu_Helpers::RadioMenuElem;
	using Gtk::Menu_Helpers::CheckMenuElem;
	using Gtk::CheckMenuItem;

	// Set up the application menu
	// The slot we use here just causes AppWindow::hide() on this,
	// which shuts down the application.
	m_menu_file.items().push_back( MenuElem("_New Game", Gtk::AccelKey( "n" ),
			sigc::mem_fun( m_viewer, &Viewer::newGame )) );
	m_menu_file.items().push_back( MenuElem("_Reset", Gtk::AccelKey( "r" ),
			sigc::mem_fun( m_viewer, &Viewer::reset )) );
	m_menu_file.items().push_back( MenuElem("_Quit", Gtk::AccelKey( "q" ),
			sigc::mem_fun( *this, &AppWindow::hide )) );

	// Set up the Draw Mode menu
	Gtk::RadioButtonGroup m_drawmodegroup;
	sigc::slot1<void, Viewer::DrawMode> drawmode_slot =
			sigc::mem_fun( m_viewer, &Viewer::set_drawmode );
	m_menu_draw_mode.items().push_back( RadioMenuElem(m_drawmodegroup,
			"_Face",
			Gtk::AccelKey( "f" ),
			sigc::bind( drawmode_slot, Viewer::FACE )) );
	m_menu_draw_mode.items().push_back( RadioMenuElem(m_drawmodegroup,
			"_Wire-frame",
			Gtk::AccelKey( "w" ),
			sigc::bind( drawmode_slot, Viewer::WIRE_FRAME )) );
	m_menu_draw_mode.items().push_back( RadioMenuElem(m_drawmodegroup,
			"_Multicoloured",
			Gtk::AccelKey( "m" ),
			sigc::bind( drawmode_slot, Viewer::MULTICOLOURED )) );

	// Set up the Speed menu
	Gtk::RadioButtonGroup m_speedgroup;
	sigc::slot1<void, Viewer::Speed> speed_slot =
			sigc::mem_fun( m_viewer, &Viewer::set_speed );
	m_menu_speed.items().push_back( RadioMenuElem(m_speedgroup, "_Slow",
			sigc::bind( speed_slot, Viewer::SLOW )) );
	m_menu_speed.items().push_back( RadioMenuElem(m_speedgroup, "_Medium",
			sigc::bind( speed_slot, Viewer::MEDIUM )) );
	m_menu_speed.items().push_back( RadioMenuElem(m_speedgroup, "_Fast",
			sigc::bind( speed_slot, Viewer::FAST )) );

	// Set up the Buffering menu
	m_menu_buffering.items().push_back( CheckMenuElem("_Double Buffering",
			Gtk::AccelKey( "b" ),
			sigc::mem_fun( m_viewer, &Viewer::swap_buffermode )) );

	// Set up the menu bar
	m_menubar.items().push_back( Gtk::Menu_Helpers::MenuElem("_File",
			m_menu_file) );
	m_menubar.items().push_back( Gtk::Menu_Helpers::MenuElem("_Draw Mode",
			m_menu_draw_mode) );
	m_menubar.items().push_back( Gtk::Menu_Helpers::MenuElem("_Speed",
			m_menu_speed) );
	m_menubar.items().push_back( Gtk::Menu_Helpers::MenuElem("_Buffering",
			m_menu_buffering) );

	// Pack in our widgets
	// First add the vertical box as our single "top" widget
	add( m_vbox );

	// Put the menubar on the top, and make it as small as possible
	m_vbox.pack_start( m_menubar, Gtk::PACK_SHRINK );

	// Put the viewer below the menubar. pack_start "grows" the widget
	// by default, so it'll take up the rest of the window.
	m_viewer.set_size_request( 300, 600 );
	m_vbox.pack_start( m_viewer );

	show_all();

	// Set double buffering to default
	static_cast<CheckMenuItem*>( &m_menu_buffering.items()[0] )->set_active();

	// Set the pointer to the window object so we can communicate back
	m_viewer.set_window(this);
}

bool AppWindow::on_key_press_event( GdkEventKey *ev )
{
	switch (ev->keyval)
	{
	case 65505: // Shift
		m_viewer.set_key( ev->keyval );
		break;
	case 65361: // Left
		m_viewer.moveLeft();
		break;
	case 65363: // Right
		m_viewer.moveRight();
		break;
	case 65362: // Up
		m_viewer.rotateCCW();
		break;
	case 65364: // Down
		m_viewer.rotateCW();
		break;
	case 32:    // Space
		m_viewer.drop();
		break;
	default:
        return Gtk::Window::on_key_press_event( ev );
	}

	m_viewer.invalidate();

    return true;
}

bool AppWindow::on_key_release_event( GdkEventKey *ev )
{
	switch (ev->keyval)
	{
	case 65505: // Shift
		m_viewer.set_key( 0 );
		break;
	default:
        return Gtk::Window::on_key_release_event( ev );
	}

	m_viewer.invalidate();

	return true;
}

void AppWindow::update_speed( int speed )
{
	using Gtk::RadioMenuItem;

	static_cast<RadioMenuItem*>( &m_menu_speed.items()[speed] )->set_active();
}
