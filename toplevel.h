/* -------------------------------------------------------------
   KDE Tuberling
   Top level window
   mailto:ebisch@cybercable.tm.fr
 ------------------------------------------------------------- */


#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_

#include <ktmainwindow.h>

class PlayGround;

class TopLevel : public KTMainWindow
{
  Q_OBJECT

public:

  TopLevel();
  ~TopLevel();

  void enableUndo(bool) const;
  void enableRedo(bool) const;

  inline bool isSoundEnabled() const {return soundEnabled;}

protected:

  void readOptions();
  void writeOptions();
  void setupMenuBar();
  void setupToolBar();

private slots:

  void fileNew();
  void fileOpen();
  void fileSave();
  void filePicture();
  void filePrint();
  void editCopy();
  void editUndo();
  void editRedo();
  void optionsSound();

private:

  KMenuBar *menubar;            // Task window's menubar
  KToolBar *toolbar;            // Task window's tool bar
  QPopupMenu *fileMenu,         // Various menus
             *editMenu,
             *optionsMenu;
  int                           // Menu items identificators
      newID, openID, saveID, pictureID, printID, quitID,
      copyID, undoID, redoID,
      soundID;
  enum {                        // Tool bar buttons identificators
      ID_NEW, ID_OPEN, ID_SAVE, ID_PRINT,
      ID_UNDO, ID_REDO,
      ID_HELP };

  bool soundEnabled;            // True if the sound is enabled by user, even if there is no audio server

  PlayGround *playGround;	// Play ground central widget
};

#endif
