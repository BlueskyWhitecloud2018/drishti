#include <GL/glew.h>

#include "global.h"
#include "staticfunctions.h"
#include "imagecaptions.h"
#include "volumeinformation.h"

#include <QGLViewer/qglviewer.h>
using namespace qglviewer;

#include <QFileDialog>
#include <QInputDialog>


ImageCaptions::ImageCaptions() { m_imageCaptions.clear(); }

ImageCaptions::~ImageCaptions() { clear(); }

void
ImageCaptions::setActive(bool b)
{
  for(int i=0; i<m_imageCaptions.count(); i++)
    m_imageCaptions[i]->setActive(b);
}

bool
ImageCaptions::isActive()
{
  bool active = false;
  for(int i=0; i<m_imageCaptions.count(); i++)
    active = active || m_imageCaptions[i]->active();

  return active;
}

void
ImageCaptions::addInMouseGrabberPool()
{
  for(int i=0; i<m_imageCaptions.count(); i++)
    m_imageCaptions[i]->addInMouseGrabberPool();
}

void
ImageCaptions::clear()
{
  for(int i=0; i<m_imageCaptions.count(); i++)
    m_imageCaptions[i]->removeFromMouseGrabberPool();

  for(int i=0; i<m_imageCaptions.count(); i++)
    delete m_imageCaptions[i];

  m_imageCaptions.clear();
  updateConnections();
}

QList<ImageCaptionObject>
ImageCaptions::imageCaptions()
{
  QList<ImageCaptionObject> caplist;
  for(int i=0; i<m_imageCaptions.count(); i++)
    caplist << m_imageCaptions[i]->imageCaption();

  return caplist;
}


bool
ImageCaptions::grabsMouse()
{
  for(int i=0; i<m_imageCaptions.count(); i++)
    {
      if (m_imageCaptions[i]->grabsMouse())
	return true;
    }
  return false;
}

void
ImageCaptions::add(Vec pt)
{
  QString imgFile = QFileDialog::getOpenFileName(0,
                                 QString("Load image/movie/text/html"),
                                 Global::previousDirectory(),
                                 "Files (*.*)");
	      
  if (imgFile.isEmpty()) return;
  QFileInfo f(imgFile);
  if (f.exists() == false) return;
  
  VolumeInformation pvlInfo = VolumeInformation::volumeInformation();
  QFileInfo fileInfo(pvlInfo.pvlFile);
  imgFile = fileInfo.absoluteDir().relativeFilePath(imgFile);

  ImageCaptionGrabber *cmg = new ImageCaptionGrabber();
  cmg->set(pt, imgFile);
  m_imageCaptions << cmg;

  updateConnections();
}

void
ImageCaptions::add(ImageCaptionObject cap)
{
  ImageCaptionGrabber *cmg = new ImageCaptionGrabber();
  cmg->setImageCaption(cap);
  m_imageCaptions << cmg;

  updateConnections();
}

void
ImageCaptions::setImageCaptions(QList<ImageCaptionObject> caps)
{
  clear();

  for(int i=0; i<caps.count(); i++)
    add(caps[i]);
}


void
ImageCaptions::postdraw(QGLViewer *viewer)
{
  int pointSize = 30;

  glEnable(GL_POINT_SPRITE);
  glActiveTexture(GL_TEXTURE0);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, Global::infoSpriteTexture());
  glTexEnvf( GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE );
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
  glEnable(GL_POINT_SMOOTH);

  glDisable(GL_DEPTH_TEST);
  viewer->startScreenCoordinatesSystem();

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // blend on top

  int wd = viewer->size().width();
  int ht = viewer->size().height();
  //--------------------
  // draw grabbed point
  glColor3f(0, 1, 0.3);
  glPointSize(pointSize+10);
  glBegin(GL_POINTS);
  for(int i=0; i<m_imageCaptions.count(); i++)
    {
      if (m_imageCaptions[i]->grabsMouse())
	{
	  Vec pt = m_imageCaptions[i]->position();
	  if (pt.z < -100000)
	    {
	      float x = pt.x * wd;
	      float y = pt.y * ht;
	      glVertex2f(x,y);
	    }
	}
    }
  glEnd();

  //--------------------

  //--------------------
  // draw active points
  glColor3f(1, 1, 1);
  glPointSize(pointSize+20);
  glBegin(GL_POINTS);
  for(int i=0; i<m_imageCaptions.count(); i++)
    {
      if (m_imageCaptions[i]->active())
	{
	  Vec pt = m_imageCaptions[i]->position();
	  if (pt.z < -100000)
	    {
	      float x = pt.x * wd;
	      float y = pt.y * ht;
	      glVertex2f(x,y);
	    }
	}
    }
  glEnd();
  //--------------------

  //--------------------
  // draw rest of the points
  glColor3f(1,1,1);
  glPointSize(pointSize);
  glBegin(GL_POINTS);
  for(int i=0; i<m_imageCaptions.count(); i++)
    {
      if (! m_imageCaptions[i]->grabsMouse() &&
	  ! m_imageCaptions[i]->active())
	{
	  Vec pt = m_imageCaptions[i]->position();
	  if (pt.z < -100000)
	    {
	      float x = pt.x * wd;
	      float y = pt.y * ht;
	      glVertex2f(x,y);
	    }
	}
    }
  glEnd();

  glPointSize(1);

  glDisable(GL_POINT_SPRITE);
  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_TEXTURE_2D);
  

  viewer->stopScreenCoordinatesSystem();
  glEnable(GL_DEPTH_TEST);
}

void
ImageCaptions::draw(QGLViewer *viewer, bool backToFront)
{
  int pointSize = 30;

  glEnable(GL_POINT_SPRITE);
  glActiveTexture(GL_TEXTURE0);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, Global::infoSpriteTexture());
  glTexEnvf( GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE );
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
  glEnable(GL_POINT_SMOOTH);


  Vec voxelScaling = Global::voxelScaling();

  //--------------------
  // draw grabbed point
  glColor3f(0, 1, 0.3);
  glPointSize(pointSize+10);
  glBegin(GL_POINTS);
  for(int i=0; i<m_imageCaptions.count(); i++)
    {
      if (m_imageCaptions[i]->grabsMouse())
	{
	  Vec pt = m_imageCaptions[i]->position();
	  if (pt.z > -100000)
	    {
	      pt = VECPRODUCT(pt, voxelScaling);
	      glVertex3fv(pt);
	    }
	}
    }
  glEnd();
  //--------------------

  //--------------------
  // draw active points
  glColor3f(1, 1, 1);
  glPointSize(pointSize+20);
  glBegin(GL_POINTS);
  for(int i=0; i<m_imageCaptions.count(); i++)
    {
      if (m_imageCaptions[i]->active())
	{
	  Vec pt = m_imageCaptions[i]->position();
	  if (pt.z > -100000)
	    {
	      pt = VECPRODUCT(pt, voxelScaling);	  
	      glVertex3fv(pt);
	    }
	}
    }
  glEnd();
  //--------------------

  //--------------------
  // draw rest of the points
  glColor3f(1,1,1);
  glPointSize(pointSize);
  glBegin(GL_POINTS);
  for(int i=0; i<m_imageCaptions.count(); i++)
    {
      if (! m_imageCaptions[i]->grabsMouse() &&
	  ! m_imageCaptions[i]->active())
	{
	  Vec pt = m_imageCaptions[i]->position();
	  if (pt.z > -100000)
	    {
	      pt = VECPRODUCT(pt, voxelScaling);
	      glVertex3fv(pt);
	    }
	}
    }
  glEnd();

  glPointSize(1);

  glDisable(GL_POINT_SPRITE);
  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_TEXTURE_2D);
  
  glDisable(GL_POINT_SMOOTH);
}

bool
ImageCaptions::keyPressEvent(QKeyEvent *event)
{
  for(int i=0; i<m_imageCaptions.count(); i++)
    {
      if (m_imageCaptions[i]->grabsMouse())
	{
	  if (event->key() == Qt::Key_Delete ||
	      event->key() == Qt::Key_Backspace ||
	      event->key() == Qt::Key_Backtab)
	    {
	      m_imageCaptions[i]->removeFromMouseGrabberPool();
	      m_imageCaptions.removeAt(i);
	      updateConnections();
	      return true;
	    }
	  if (event->key() == Qt::Key_S)
	    {
	      m_imageCaptions[i]->setActive(false);
	      m_imageCaptions[i]->saveSize();
	    }
	  else if (event->key() == Qt::Key_Space)
	    {
	      Vec pos = m_imageCaptions[i]->position();
	      bool ok;
	      QString text;
	      if (pos.z > -100000)
		text = QString("%1 %2 %3").arg(pos.x).arg(pos.y).arg(pos.z);
	      else
		text = QString("%1 %2").arg(pos.x).arg(pos.y);

	      text = QInputDialog::getText(0,
					   "Coordinates",
					   "Coordinates",
					   QLineEdit::Normal,
					   text,
					   &ok);
	      if (ok && !text.isEmpty())
		{
		  float x,y,z;
		  x=y=0;
		  z=-1000000;
		  QStringList list = text.split(" ", QString::SkipEmptyParts);
		  if (list.count() > 0) x = list[0].toFloat();
		  if (list.count() > 1) y = list[1].toFloat();
		  if (list.count() > 2) z = list[2].toFloat();
		  m_imageCaptions[i]->setPosition(Vec(x,y,z));
		}
	    }
	  else if (event->key() == Qt::Key_L)
	    {
	      ImageCaptionObject* cmg = m_imageCaptions[i];
	      QString imgFile = QFileDialog::getOpenFileName(0,
                                 QString("Load image/movie/text/html"),
                                 Global::previousDirectory(),
                                 "Files (*.*)");	      
	      if (imgFile.isEmpty())
		return false;
	      
	      QFileInfo f(imgFile);
	      if (f.exists() == false)
		return false;

	      VolumeInformation pvlInfo = VolumeInformation::volumeInformation();
	      QFileInfo fileInfo(pvlInfo.pvlFile);
	      imgFile = fileInfo.absoluteDir().relativeFilePath(imgFile);

	      cmg->setImageFileName(imgFile);

	      QMessageBox::information(0, "Information Captions",
				       "Once you have set the size for the information display, to fix the size for subsequent displays, hover over information icon and press \"s\" to save size.");
	    }
	}
    }
  
  return true;
}

void
ImageCaptions::updateConnections()
{
  for(int i=0; i<m_imageCaptions.count(); i++)
    {
      m_imageCaptions[i]->disconnect();
    }
  for(int i=0; i<m_imageCaptions.count(); i++)
    {
      connect(m_imageCaptions[i], SIGNAL(activated()),
	      this, SLOT(activated()));
    }
}

void
ImageCaptions::activated()
{
  for(int i=0; i<m_imageCaptions.count(); i++)
    {
      if (!m_imageCaptions[i]->pressed())
	m_imageCaptions[i]->setActive(false);
    }
}
