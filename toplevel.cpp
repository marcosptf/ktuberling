/***************************************************************************
 *   Copyright (C) 1999-2006 by Éric Bischoff <ebischoff@nerim.net>        *
 *   Copyright (C) 2007 by Albert Astals Cid <aacid@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/* Top level window */

#include <kapplication.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kaction.h>
#include <kstandardaction.h>
#include <kstandardshortcut.h>
#include <kstandardgameaction.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <ktogglefullscreenaction.h>
#include <kimageio.h>
#include <kmimetype.h>
#include <kconfiggroup.h>

#include <QClipboard>
#include <QPrintDialog>
#include <QPrinter>

#include "toplevel.moc"
#include "playground.h"
#include "soundfactory.h"

// Constructor
TopLevel::TopLevel()
  : KXmlGuiWindow(0)
{
  QString board, language;

  playGround = new PlayGround(this);
  playGround->setObjectName( "playGround" );

  soundFactory = new SoundFactory(this);

  setCentralWidget(playGround);

  playgroundsGroup = new QActionGroup(this);
  playgroundsGroup->setExclusive(true);

  languagesGroup = new QActionGroup(this);
  languagesGroup->setExclusive(true);

  setupKAction();

  playGround->registerPlayGrounds();
  soundFactory->registerLanguages();

  readOptions(board, language);
  changeGameboard(board);
  changeLanguage(language);
}

// Destructor
TopLevel::~TopLevel()
{
  delete soundFactory;
}

// Register an available gameboard
void TopLevel::registerGameboard(const QString &menuText, const QString &board)
{
  QList<QAction*> actionList;
  KToggleAction *t = new KToggleAction(menuText, this);
  actionCollection()->addAction(board, t);
  t->setData(board);
  connect(t, SIGNAL(toggled(bool)), SLOT(changeGameboard()));
  actionList << t;
  playgroundsGroup->addAction(t);
  plugActionList( "playgroundList", actionList );
}

// Register an available language
void TopLevel::registerLanguage(const QString &code, const QString &soundFile, bool enabled)
{
  QList<QAction*> actionList;
  KToggleAction *t = new KToggleAction(KGlobal::locale()->languageCodeToName(code), this);
  t->setEnabled(enabled);
  actionCollection()->addAction(soundFile, t);
  t->setData(soundFile);
  sounds.insert(code, soundFile);
  connect(t, SIGNAL(toggled(bool)), SLOT(changeLanguage()));
  actionList << t;
  languagesGroup->addAction(t);
  plugActionList( "languagesList", actionList );
}

// Switch to another gameboard
void TopLevel::changeGameboard()
{
  QAction *action = qobject_cast<QAction*>(sender());
  // ignore toggling of "nonchecked" actions
  if (action->isChecked())
  {
    QString newGameBoard = action->data().toString();
    changeGameboard(newGameBoard);
  }
}

void TopLevel::changeGameboard(const QString &newGameBoard)
{
  if (newGameBoard == playGround->currentGameboard()) return;

  QString fileToLoad;
  QFileInfo fi(newGameBoard);
  if (fi.isRelative())
  {
    QStringList list = KGlobal::dirs()->findAllResources("appdata", "pics/" + newGameBoard);
    if (!list.isEmpty()) fileToLoad = list.first();
  }
  else
  {
    fileToLoad = newGameBoard;
  }

  if (playGround->loadPlayGround(fileToLoad))
  {
    actionCollection()->action(fileToLoad)->setChecked(true);

    // Change gameboard in the remembered options
    writeOptions();
  }
  else
  {
    KMessageBox::error(this, i18n("Error while loading the playground."));
  }
}

void TopLevel::changeLanguage()
{
  QAction *action = qobject_cast<QAction*>(sender());
  QString soundFile = action->data().toString();
  changeLanguage(soundFile);
}

// Switch to another language
void TopLevel::changeLanguage(const QString &soundFile)
{
  if (soundFile == soundFactory->currentSoundFile()) return;

  QString fileToLoad;
  QFileInfo fi(soundFile);
  if (fi.isRelative())
  {
    QStringList list = KGlobal::dirs()->findAllResources("appdata", "sounds/" + soundFile);
    if (!list.isEmpty()) fileToLoad = list.first();
  }
  else
  {
    fileToLoad = soundFile;
  }

  // Change language effectively
  if (soundFactory->loadLanguage(fileToLoad))
  {
    actionCollection()->action(fileToLoad)->setChecked(true);

    // Change language in the remembered options
    writeOptions();
  }
  else
  {
    KMessageBox::error(this, i18n("Error while loading the sound file."));
    soundOff();
  }
}

// Play a sound
void TopLevel::playSound(const QString &ref) const
{
  soundFactory->playSound(ref);
}

// Read options from preferences file
void TopLevel::readOptions(QString &board, QString &language)
{
  QString option;
  KConfigGroup config(KGlobal::config(), "General");

  option = config.readEntry("Sound", "on");
  bool soundEnabled = option.indexOf("on") == 0;

  board = config.readEntry("Gameboard", "default_theme.theme");
  if (soundEnabled)
  {
    language = config.readEntry("Language", "");
    if (language.isEmpty())
    {
      language = sounds.value(KGlobal::locale()->language(), "en.soundtheme");
    }
  }
  else
  {
    soundOff();
    language = QString();
  }
}

// Write options to preferences file
void TopLevel::writeOptions()
{
  KConfigGroup config(KGlobal::config(), "General");
  config.writeEntry("Sound", actionCollection()->action("speech_no_sound")->isChecked() ? "off": "on");

  config.writeEntry("Gameboard", playGround->currentGameboard());

  config.writeEntry("Language", soundFactory->currentSoundFile());
}

// KAction initialization (aka menubar + toolbar init)
void TopLevel::setupKAction()
{
  QAction *action;

  //Game
  KStandardGameAction::gameNew(this, SLOT(fileNew()), actionCollection());
  KStandardGameAction::load(this, SLOT(fileOpen()), actionCollection());
  KStandardGameAction::save(this, SLOT(fileSave()), actionCollection());
  KStandardGameAction::print(this, SLOT(filePrint()), actionCollection());
  KStandardGameAction::quit(kapp, SLOT(quit()), actionCollection());

  action = actionCollection()->addAction("game_save_picture");
  action->setText(i18n("Save &as Picture..."));
  connect(action, SIGNAL(triggered(bool) ), SLOT(filePicture()));

  //Edit
  action = KStandardAction::copy(this, SLOT(editCopy()), actionCollection());
  actionCollection()->addAction(action->objectName(), action);

  playGround->createRedoAction(actionCollection());
  playGround->createUndoAction(actionCollection());

  //Speech
  KToggleAction *t = new KToggleAction(i18n("&No Sound"), this);
  actionCollection()->addAction("speech_no_sound", t);
  connect(t, SIGNAL(triggered(bool) ), SLOT(soundOff()));
  languagesGroup->addAction(t);

  KStandardAction::fullScreen(this, SLOT(toggleFullScreen()), this, actionCollection());

  setupGUI(ToolBar | Keys | Save | Create);
}

void TopLevel::saveNewToolbarConfig()
{
  // this destroys our actions lists ...
  KXmlGuiWindow::saveNewToolbarConfig();
  // ... so plug them again
  plugActionList( "playgroundList", playgroundsGroup->actions() );
  plugActionList( "languagesList", languagesGroup->actions() );
}

// Reset gameboard
void TopLevel::fileNew()
{
  playGround->reset();
}

// Load gameboard
void TopLevel::fileOpen()
{
  QString dir = KStandardDirs::locate("data", "ktuberling/museum/miss.tuberling");
  dir.truncate(dir.lastIndexOf('/') + 1);

  KUrl url = KFileDialog::getOpenUrl(dir, "*.tuberling");

  open(url);
}

void TopLevel::open(const KUrl &url)
{
  if (url.isEmpty())
    return;

  QString name;

  KIO::NetAccess::download(url, name, this);

  playGround->reset();

  switch(playGround->loadFrom(name))
  {
    case PlayGround::NoError:
     // good
    break;

    case PlayGround::OldFileVersionError:
      KMessageBox::error(this, i18n("The saved file is from an old version of KTuberling and unfortunately can not be open with this version."));
    break;

    case PlayGround::OtherError:
      KMessageBox::error(this, i18n("Could not load file."));
    break;
  }
    

  KIO::NetAccess::removeTempFile( name );
}

// Save gameboard
void TopLevel::fileSave()
{
  KUrl url = KFileDialog::getSaveUrl
                ( KUrl(),
		 "*.tuberling");

  if (url.isEmpty())
    return;

  if( !url.isLocalFile() )
  {
    KMessageBox::sorry(this,
                       i18n("Only saving to local files is currently "
                            "supported."));
    return;
  }

  QString name = url.path();
  int suffix;

  suffix = name.lastIndexOf('.');
  if (suffix == -1)
  {
    name += ".tuberling";
  }

  if( !playGround->saveAs( name ) )
    KMessageBox::error(this, i18n("Could not save file."));
}

// Save gameboard as picture
void TopLevel::filePicture()
{
  KUrl url = KFileDialog::getSaveUrl(KUrl(), KImageIO::pattern(KImageIO::Writing));

  if( url.isEmpty() )
    return;

  if( !url.isLocalFile() )
  {
    KMessageBox::sorry(this,
                       i18n("Only saving to local files is currently "
                            "supported."));
    return;
  }

  KMimeType::Ptr mime = KMimeType::findByUrl(url, 0, true, true);
  if (!KImageIO::isSupported(mime->name(), KImageIO::Writing))
  {
    KMessageBox::error(this, i18n("Unknown picture format."));
    return;
  };

  QStringList types = KImageIO::typeForMime(mime->name());
  if (types.isEmpty()) return; // TODO error dialog?

  QPixmap picture(playGround->getPicture());

  QString name = url.path();

  if (!picture.save(name, types.at(0).toLatin1()))
    KMessageBox::error
      (this, i18n("Could not save file."));
}

// Save gameboard as picture
void TopLevel::filePrint()
{
  QPrinter printer;
  bool ok;
  
  QPrintDialog printDialog(&printer, this);
  printDialog.setWindowTitle(i18n("Print %1", actionCollection()->action(playGround->currentGameboard())->iconText()));
  ok = printDialog.exec();
  if (!ok) return;
  playGround->repaint();
  if (!playGround->printPicture(printer))
    KMessageBox::error(this,
                         i18n("Could not print picture."));
  else
    KMessageBox::information(this,
                             i18n("Picture successfully printed."));
}

// Copy modified area to clipboard
void TopLevel::editCopy()
{
  QClipboard *clipboard = QApplication::clipboard();
  QPixmap picture(playGround->getPicture());

  clipboard->setPixmap(picture);
}

// Toggle sound off
void TopLevel::soundOff()
{
  actionCollection()->action("speech_no_sound")->setChecked(true);
  writeOptions();
}

bool TopLevel::isSoundEnabled() const
{
  return !actionCollection()->action("speech_no_sound")->isChecked();
}

void TopLevel::toggleFullScreen()
{
  KToggleFullScreenActive::setFullScreen( this, actionCollection()->action("fullscreen")->isChecked());
}
