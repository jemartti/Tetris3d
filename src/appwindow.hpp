#ifndef APPWINDOW_HPP
#define APPWINDOW_HPP

#include <gtkmm.h>

#include "viewer.hpp"

class AppWindow : public Gtk::Window {
public:
	AppWindow();

	// Updates the speed menu radio buttons
	void update_speed  ( int speed );
  
protected:
	// Handle I/O
	virtual bool on_key_press_event  ( GdkEventKey *ev );
	virtual bool on_key_release_event( GdkEventKey *ev );

private:
	// A "vertical box" which holds everything in our window
	Gtk::VBox    m_vbox;

	// The menubar, with all the menus at the top of the window
	Gtk::MenuBar m_menubar;

	// Each menu itself
	Gtk::Menu    m_menu_file;
	Gtk::Menu    m_menu_draw_mode;
	Gtk::Menu    m_menu_speed;
	Gtk::Menu    m_menu_buffering;

	// The main OpenGL area
	Viewer       m_viewer;
};

#endif
