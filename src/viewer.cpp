#include "viewer.hpp"
#include "appwindow.hpp"

#include <GL/gl.h>
#include <GL/glu.h>

#include <iostream>


Viewer::Viewer()
{
	Glib::RefPtr<Gdk::GL::Config> glconfig;

	// Ask for an OpenGL Setup with
	//  - red, green and blue component colour
	//  - a depth buffer to avoid things overlapping wrongly
	//  - double-buffered rendering to avoid tearing/flickering
	glconfig = Gdk::GL::Config::create( Gdk::GL::MODE_RGB   |
										Gdk::GL::MODE_DEPTH |
										Gdk::GL::MODE_DOUBLE );
	if (glconfig == 0) {
		// If we can't get this configuration, die
		std::cerr << "Unable to setup OpenGL Configuration!" << std::endl;
		abort();
	}

	// Accept the configuration
	set_gl_capability( glconfig );

	// Register the fact that we want to receive these events
	add_events( Gdk::BUTTON1_MOTION_MASK    |
				Gdk::BUTTON2_MOTION_MASK    |
				Gdk::BUTTON3_MOTION_MASK    |
				Gdk::BUTTON_PRESS_MASK      |
				Gdk::BUTTON_RELEASE_MASK    |
				Gdk::BUTTON_MOTION_MASK     |
				Gdk::POINTER_MOTION_MASK    |
				Gdk::VISIBILITY_NOTIFY_MASK );


	m_game       = NULL;

	m_speed      = SLOW;
	m_drawmode   = FACE;
	m_buffermode = SINGLE;

	m_ixpos      = 0.0;
	m_xpos       = 0.0;
	m_button1    = false;
	m_button2    = false;
	m_button3    = false;

	m_key        = 0;

	m_scalef     = 1.0;
	m_rotx       = 0.0;
	m_roty       = 0.0;
	m_rotz       = 0.0;
	m_xdir       = 1;
	m_ydir       = 1;
	m_zdir       = 1;

	m_rowCount   = 0;

	m_x1         = 0.0;
	m_y1         = 0.0;
	m_z1         = 0.0;

	m_tick       = 500;

	m_persist    = false;
}

Viewer::~Viewer()
{
	m_gameTiming.disconnect();
	free(m_game);
}

void Viewer::set_drawmode( DrawMode drawmode )
{
	m_drawmode = drawmode;
	invalidate();
}

void Viewer::set_speed( Speed speed )
{
	m_speed = speed;
	update_speed( speed );
}

void Viewer::set_key( int key )
{
	m_key = key;
}

void Viewer::set_window( AppWindow* window )
{
	m_window = window;
}

Game* Viewer::get_game()
{
	return m_game;
}

void Viewer::invalidate()
{
	//Force a re-render
	Gtk::Allocation allocation = get_allocation();
	get_window()->invalidate_rect( allocation, false);
}

void Viewer::on_realize()
{
	// Do some OpenGL setup.
	// First, let the base class do whatever it needs to
	Gtk::GL::DrawingArea::on_realize();

	Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

	if ( !gldrawable )
	{
		return;
	}

	if ( !gldrawable->gl_begin(get_gl_context()) )
	{
		return;
	}

	// Just enable depth testing and set the background colour.
	glEnable( GL_DEPTH_TEST );
	glClearColor( 0.7, 0.7, 1.0, 0.0 );

	gldrawable->gl_end();
}

bool Viewer::on_expose_event( GdkEventExpose* /*event*/ )
{
	Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

	if ( !gldrawable )
	{
		return false;
	}

	if ( !gldrawable->gl_begin(get_gl_context()) )
	{
		return false;
	}
	// Clear the screen
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Modify the current projection matrix so that we move the
	// camera away from the origin.  We'll draw the game at the
	// origin, and we need to back up to see it.
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glTranslated( 0.0, 0.0, -40.0 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// Not implemented: set up lighting (if necessary)

	// Set buffer mode
	if ( m_buffermode == SINGLE )
	{
	  glDrawBuffer( GL_FRONT );
	}
	else
	{
	  glDrawBuffer( GL_BACK );
	}

	// Scale the scene
	if ( m_key == 65505 )
	{
		if ( m_button1 || m_button2 || m_button3 )
		{
			double scalef = m_scalef + ( m_xpos - m_ixpos ) / 100;
			m_ixpos       = m_xpos;
			if ( (scalef > 0.05) && (scalef < 1.95) )
			{
				m_scalef = scalef;
			}
		}
	}
	glScaled( m_scalef, m_scalef, m_scalef );

	// Rotate the scene
	if ( m_button1 )
	{
		double rotx = m_rotx + ( m_xpos - m_ixpos );
		if ( (m_ixpos != m_xpos) && (( m_xpos - m_ixpos ) < 0) )
		{
			m_xdir = -1;
		}
		else if ( m_ixpos != m_xpos )
		{
			m_xdir = 1;
		}
		m_rotx  = rotx;
	}
	if ( m_button2 )
	{
		double roty = m_roty + ( m_xpos - m_ixpos );
		if ( (m_ixpos != m_xpos) && (( m_xpos - m_ixpos ) < 0) )
		{
			m_ydir = -1;
		}
		else if ( m_ixpos != m_xpos )
		{
			m_ydir = 1;
		}
		m_roty  = roty;
	}
	if ( m_button3 )
	{
		double rotz = m_rotz + ( m_xpos - m_ixpos );
		if ( (m_ixpos != m_xpos) && (( m_xpos - m_ixpos ) < 0) )
		{
			m_zdir = -1;
		}
		else if ( m_ixpos != m_xpos )
		{
			m_zdir = 1;
		}
		m_rotz  = rotz;
	}
	if ( m_persist )
	{
		if ( m_button1 )
		{
			m_rotx = ( m_rotx + (0.5 * m_xdir) ) - static_cast<double>(
					static_cast<int>(( m_rotx + (0.5 * m_xdir) ) / 360) ) * 360;
		}
		if ( m_button2 )
		{
			m_roty = ( m_roty + (0.5 * m_ydir) ) - static_cast<double>(
					static_cast<int>(( m_roty + (0.5 * m_ydir) ) / 360) ) * 360;
		}
		if ( m_button3 )
		{
			m_rotz = ( m_rotz + (0.5 * m_zdir) ) - static_cast<double>(
					static_cast<int>(( m_rotz + (0.5 * m_zdir) ) / 360) ) * 360;
		}
		glRotated( m_rotx, 1.0, 0.0, 0.0 );
		glRotated( m_roty, 0.0, 1.0, 0.0 );
		glRotated( m_rotz, 0.0, 0.0, 1.0 );
	}
	else
	{
		m_ixpos = m_xpos;
		glRotated( m_rotx, 1.0, 0.0, 0.0 );
		glRotated( m_roty, 0.0, 1.0, 0.0 );
		glRotated( m_rotz, 0.0, 0.0, 1.0 );
	}

	// You'll be drawing unit cubes, so the game will have width
	// 10 and height 24 (game = 20, stripe = 4).  Let's translate
	// the game so that we can draw it starting at (0,0) but have
	// it appear centred in the window.
	glTranslated( -5.0, -12.0, 0.0 );

	// Set up the draw mode
	if ( m_drawmode == Viewer::WIRE_FRAME )
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	}
	else
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	}

	drawGame();

	// We pushed a matrix onto the PROJECTION stack earlier, we
	// need to pop it.
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	// Swap the contents of the front and back buffers so we see what we
	// just drew. This should only be done if double buffering is enabled.
	if ( m_buffermode == DOUBLE )
	{
		gldrawable->swap_buffers();
	}
	else
	{
		glFlush();
	}

	gldrawable->gl_end();

	return true;
}

bool Viewer::on_configure_event( GdkEventConfigure* event )
{
	Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

	if ( !gldrawable )
	{
		return false;
	}

	if ( !gldrawable->gl_begin(get_gl_context()) )
	{
		return false;
	}

	// Set up perspective projection, using current size and aspect
	// ratio of display
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glViewport( 0, 0, event->width, event->height );
	gluPerspective( 40.0, (GLfloat)event->width / (GLfloat)event->height,
		  0.1, 1000.0 );

	// Reset to modelview matrix mode
	glMatrixMode( GL_MODELVIEW );

	gldrawable->gl_end();

	return true;
}

bool Viewer::on_button_press_event( GdkEventButton* event )
{
	if ( m_persist )
	{
		m_button1 = false;
		m_button2 = false;
		m_button3 = false;
	}
	switch (event->button)
	{
	case 1:
		m_button1 = true;
		break;
	case 2:
		m_button2 = true;
		break;
	case 3:
		m_button3 = true;
		break;
	default:
		break;
	}
	m_ixpos   = event->x;
	m_xpos    = event->x;
	m_persist = false;
	m_rotateTiming.disconnect();
	invalidate();
	return true;
}

bool Viewer::on_button_release_event( GdkEventButton* /*event*/ )
{
	gettimeofday( &m_curtime, NULL );
	if ( !m_key && abs(m_curtime.tv_usec - m_lasttime.tv_usec) < 10000 )
	{
		m_persist = true;
		m_rotateTiming = Glib::signal_timeout().connect(
				sigc::mem_fun(*this, &Viewer::inval_sig), 3 );
	}
	else
	{
		m_persist = false;
		m_rotateTiming.disconnect();
		m_button1 = false;
		m_button2 = false;
		m_button3 = false;
	}

	m_ixpos  = 0.0;
	m_xpos   = 0.0;
	return true;
}

bool Viewer::on_motion_notify_event(GdkEventMotion* event)
{
	if ( (m_button1 || m_button2 || m_button3) && !m_persist )
	{
		m_xpos   = event->x;
		gettimeofday( &m_lasttime, NULL );
	}
	invalidate();
	return true;
}

void Viewer::drawCube( double x, double y, double z, int type )
{
	glBegin(GL_QUADS);

	// Set which type of game piece we have
	switch (type)
	{
	case 0:
		m_x1 = 0.0;
		m_y1 = 1.0;
		m_z1 = 0.0;
		break;
	case 1:
		m_x1 = 1.0;
		m_y1 = 0.5;
		m_z1 = 0.0;
		break;
	case 2:
		m_x1 = 1.0;
		m_y1 = 0.0;
		m_z1 = 0.0;
		break;
	case 3:
		m_x1 = 1.0;
		m_y1 = 1.0;
		m_z1 = 0.0;
		break;
	case 4:
		m_x1 = 0.0;
		m_y1 = 0.0;
		m_z1 = 1.0;
		break;
	case 5:
		m_x1 = 1.0;
		m_y1 = 0.0;
		m_z1 = 1.0;
		break;
	case 6:
		m_x1 = 1.0;
		m_y1 = 1.0;
		m_z1 = 1.0;
		break;
	default:
		m_x1 = 0.0;
		m_y1 = 0.0;
		m_z1 = 0.0;
		break;
	}
	glColor3d( m_x1, m_y1, m_z1 );

	// Draw the front
	if ( m_drawmode == MULTICOLOURED )
	{
		set_Multi();
	}
	glVertex3d( x+1.0, y+1.0, z     ); // Front
	glVertex3d( x,     y+1.0, z     );
	glVertex3d( x,     y,     z     );
	glVertex3d( x+1.0, y,     z     );

	// Draw the back
	if ( m_drawmode == MULTICOLOURED )
	{
		set_Multi();
	}
	glVertex3d( x,     y+1.0, z-1.0 ); // Back
	glVertex3d( x+1.0, y+1.0, z-1.0 );
	glVertex3d( x+1.0, y,     z-1.0 );
	glVertex3d( x,     y,     z-1.0 );

	// Draw the top
	if ( m_drawmode == MULTICOLOURED )
	{
		set_Multi();
	}
	// Make sides of wells grey if not in multicoloured mode
	else if ( type == -1 )
	{
		m_x1 = 0.2;
		m_y1 = 0.2;
		m_z1 = 0.2;
		glColor3d( m_x1, m_y1, m_z1 );
	}
	glVertex3d( x+1.0, y+1.0, z-1.0 ); // Top
	glVertex3d( x,     y+1.0, z-1.0 );
	glVertex3d( x,     y+1.0, z     );
	glVertex3d( x+1.0, y+1.0, z     );

	// Draw the bottom
	if ( m_drawmode == MULTICOLOURED )
	{
		set_Multi();
	}
	glVertex3d( x+1.0, y,     z     ); // Bottom
	glVertex3d( x,     y,     z     );
	glVertex3d( x,     y,     z-1.0 );
	glVertex3d( x+1.0, y,     z-1.0 );

	// Draw the left
	if ( m_drawmode == MULTICOLOURED )
	{
		set_Multi();
	}
	glVertex3d( x, y+1.0, z         ); // Left
	glVertex3d( x, y+1.0, z-1.0     );
	glVertex3d( x, y,     z-1.0     );
	glVertex3d( x, y,     z         );

	// Draw the right
	if ( m_drawmode == MULTICOLOURED )
	{
		set_Multi();
	}
	glVertex3d( x+1.0, y+1.0, z-1.0 ); // Right
	glVertex3d( x+1.0, y+1.0, z     );
	glVertex3d( x+1.0, y,     z     );
	glVertex3d( x+1.0, y,     z-1.0 );

	glEnd();
}

void Viewer::set_Multi()
{
	// Calculate the next colour based on the previous colour
	// Basic idea is a base 3 number system
	if (m_x1 < 1.0)
	{
		m_x1 += 0.5;
	}
	else
	{
		if (m_y1 < 1.0)
		{
			m_y1 += 0.5;
		}
		else
		{
			if (m_z1 < 1.0)
			{
				m_z1 += 0.5;
			}
			else
			{
				m_x1 = 0;
				m_y1 = 0;
				m_z1 = 0;
			}
		}
	}

	// Set the draw colour
	glColor3d( m_x1, m_y1, m_z1 );
}

void Viewer::drawGame()
{
	// Draw well
	for ( double i = -1.0; i < 20.0; i+=1.0 )
	{
		drawCube( -1.0, i, 0.0, -1 );
		drawCube( 10.0, i, 0.0, -1 );
	}
	// Draws the bottom wall
	for ( double i = 0.0; i < 10.0; i+=1.0 )
	{
		drawCube( i, -1.0, 0.0, -1 );
	}

	// Draw pieces
	int piece;
	if ( m_game != NULL )
	{
		for ( int i = 0; i < 24; i++ )
		{
			for ( int j = 0; j < 10; j++ )
			{
				piece = m_game->get( i, j );
				if ( piece >= 0 )
				{
					drawCube( j, i, 0.0, piece );
				}
			}
		}
	}
}

int Viewer::execute_tick()
{
	if ( m_game == NULL )
	{
		return 1;
	}
	int rows = m_game->tick();
	invalidate();
	m_rowCount += rows;
	if      ( m_rowCount >= 20 )
	{
		update_speed( FAST );
	}
	else if ( m_rowCount >= 10 )
	{
		update_speed( MEDIUM );
	}

	return 1;
}

void Viewer::update_speed( Speed speed )
{
	if ( speed == FAST )
	{
		m_rowCount = 20;
		m_tick = 100;
	}
	else if ( speed == MEDIUM )
	{
		m_rowCount = 10;
		m_tick = 300;
	}
	else
	{
		m_rowCount = 0;
		m_tick = 500;
	}
	m_gameTiming.disconnect();
	m_gameTiming = Glib::signal_timeout().connect(
			sigc::mem_fun(*this, &Viewer::execute_tick), m_tick );
	if ( m_speed != speed )
	{
		m_speed      = speed;
		m_window->update_speed( (int)speed );
	}
}

int Viewer::inval_sig()
{
	invalidate();
	return 1;
}

void Viewer::swap_buffermode()
{
	m_buffermode = (BufferMode)( ((int)m_buffermode + 1) % 2 );
	invalidate();
}

void Viewer::reset()
{
	m_ixpos      = 0.0;
	m_xpos       = 0.0;
	m_button1    = false;
	m_button2    = false;
	m_button3    = false;

	m_key        = 0;

	m_scalef     = 1.0;
	m_rotx       = 0.0;
	m_roty       = 0.0;
	m_rotz       = 0.0;

	m_x1         = 0.0;
	m_y1         = 0.0;
	m_z1         = 0.0;
	invalidate();
}

void Viewer::newGame()
{
	update_speed( SLOW );
	free(m_game);
	m_game     = new Game::Game( 10, 20 );
	m_game->reset();
	m_gameTiming.disconnect();
	m_gameTiming = Glib::signal_timeout().connect(
			sigc::mem_fun(*this, &Viewer::execute_tick), m_tick );
	invalidate();
}

void Viewer::moveLeft()
{
	if ( m_game )
	{
		m_game->moveLeft();
	}
}

void Viewer::moveRight()
{
	if ( m_game )
	{
		m_game->moveRight();
	}
}

void Viewer::rotateCCW()
{
	if ( m_game )
	{
		m_game->rotateCCW();
	}
}

void Viewer::rotateCW()
{
	if ( m_game )
	{
		m_game->rotateCW();
	}
}

void Viewer::drop()
{
	if ( m_game )
	{
		m_game->drop();
	}
}
