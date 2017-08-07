#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#define DOSYA_ACILMADI (dosya_adi[0] == '\0')

char *PROGNAME;
char *program_basligi = "İsimsiz - GTK Not Defteri";
char *ikon  = "ikon.png";
char *dosya_adi = NULL;

// Ekran
GtkWidget *window, *vbox;
GtkWindow *gwindow;
GtkAccelGroup *accel = NULL;
GtkWidget *statusbar;

GdkAtom atom;
GtkClipboard *clipboard;

// Menubar
GtkWidget *menubar, *sep;

// Dosya Menüsü
GtkWidget *filemenu, *file;
GtkWidget *new, *open, *save, *saveas, *quit;

// Düzenle Menüsü
GtkWidget *editmenu, *edit;
GtkWidget *cut, *copy, *paste, *delete;
GtkWidget *selectall, *time_date;
GtkWidget *wll, *select_font;

// Yardım Menüsü
GtkWidget *helpmenu, *help;
GtkWidget *about;

// Yazı Alanı
GtkWidget *textarea;
GtkTextBuffer *buffer;
GtkWidget *kaydirilabilir_ekran;
gboolean degisiklik = FALSE;
char cumle_bolumu = 1;

void notepad_pozisyon_guncelle(void);
char notepad_dosya_ac(const char *);
void notepad_kaydet();
void dosyayi_farkli_kaydet(void);