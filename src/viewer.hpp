#ifndef CS488_VIEWER_HPP
#define CS488_VIEWER_HPP

#include <gtkglmm.h>
#include <gtkmm.h>

#include <sys/time.h>

#include "game.hpp"

class AppWindow;

// The "main" OpenGL widget
class Viewer : public Gtk::GL::DrawingArea {
public:
	// Enums for game state
	enum DrawMode {
		WIRE_FRAME,
		FACE,
		MULTICOLOURED
	};
	enum BufferMode {
		SINGLE,
		DOUBLE
	};
	enum Speed {
		SLOW,
		MEDIUM,
		FAST
	};

	Viewer();
	virtual ~Viewer();

	// Setter functions
	void set_drawmode( DrawMode   drawmode );
	void set_speed   ( Speed      speed    );
	void set_key     ( int        key      );
	void set_window  ( AppWindow* window   );

	// Getter functions
	Game* get_game ();

	// A useful function that forces this widget to rerender. If you
	// want to render a new frame, do not call on_expose_event
	// directly. Instead call this, which will cause an on_expose_event
	// call when the time is right.
	void invalidate();

	// Utility functions for updating game state
	void swap_buffermode();
	void reset();
	void newGame();

	// Game control
	void moveLeft();
	void moveRight();
	void rotateCCW();
	void rotateCW();
	void drop();

protected:
	// Events we implement
	// Note that we could use gtkmm's "signals and slots" mechanism
	// instead, but for many classes there's a convenient member
	// function one just needs to define that'll be called with the
	// event.

	// Called when GL is first initialized
	virtual void on_realize();
	// Called when our window needs to be redrawn
	virtual bool on_expose_event(GdkEventExpose* event);
	// Called when the window is resized
	virtual bool on_configure_event(GdkEventConfigure* event);
	// Called when a mouse button is pressed
	virtual bool on_button_press_event(GdkEventButton* event);
	// Called when a mouse button is released
	virtual bool on_button_release_event(GdkEventButton* event);
	// Called when the mouse moves
	virtual bool on_motion_notify_event(GdkEventMotion* event);

private:
	// Used to draw a unit cube
	void drawCube( double x, double y, double z, int type );
	// Makes the colours of the unit cube assorted
	void set_Multi();

	// Used to draw the current game state
	void drawGame();

	// Move clock by one tick
	int  execute_tick();

	// Update the game speed
	void update_speed( Speed speed );

	// Invalidate function for signals
	int inval_sig();

	// The AppWindow object
	AppWindow*       m_window;

	// The game object
	Game*            m_game;

	// Timer connection for advancing gameplay
	sigc::connection m_gameTiming;

	// Current game state
	Speed            m_speed;
	DrawMode         m_drawmode;
	BufferMode       m_buffermode;

	// Mouse info
	double           m_ixpos;
	double           m_xpos;
	bool             m_button1;
	bool             m_button2;
	bool             m_button3;

	// Key info
	int              m_key;

	// Used for maintaining/calculating scale/rotation
	double           m_scalef;
	double           m_rotx;
	double           m_roty;
	double           m_rotz;
	int              m_xdir;
	int              m_ydir;
	int              m_zdir;

	// Number of cleared rows, used to decide game speed
	int              m_rowCount;

	// Used for calculating assorted colours
	double           m_x1;
	double           m_y1;
	double           m_z1;

	// Game tick time
	int              m_tick;

	// Used to decide whether rotation should have persistence
	bool             m_persist;
	timeval          m_lasttime;
	timeval          m_curtime;
	sigc::connection m_rotateTiming;

	int test();
};

#endif
