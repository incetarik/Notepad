#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <gdk/gdk.h>
#include "gtk-notepad.h"


void kullanimi_yazdir(void) {
    fprintf(stderr, "%s: usage: %s [file]\n",
            PROGNAME,
            PROGNAME);
}


/**
 * Notepad başlığını değiştirir
 * @param dosya_adi Dosya adı, eğer bir dosya açıldıysa onu görüntülemek için
 */
void notepad_basligi_degistir(const char *dosya_adi) {
    // Hafızaya dosya adı kadar yer aç
    char *fn = malloc(strlen(dosya_adi) + 1);

    // Dosya adını açılan hafızaya kot
    strcpy(fn, dosya_adi);

    // Dosya adını / ile bölme işlemi
    int i = 0;
    int index = 0;
    char *slash = strrchr(dosya_adi, '/');
    if (slash) {
        // Eğer slash varsa buradayız, pozisyonu bul (index) ve dosya adından çıkar => / den sonrası kalacak
        index = (int)(slash - dosya_adi);

        // fn++ 'ya dikkat, fn bir char* yani bir string, dosya adının üstünde geziyoruz
        for (i=0; i <= index; i++, fn++);
    }

    // ' - GTK Not Defteri' ve daha önceki dosya adı uzunluğunca yer aç
    char *baslik = malloc(strlen(" - GTK Not Defteri") + strlen(fn) + 1);

    // İçeriğini doldur
    strcpy(baslik, fn);

    // Slash varsa, yani yukarıda gezinti yapmıştık, onu geriye al
    if (slash)
        for (i=0; i <= index; i++, fn--);

    // Başlık ile '- GTK Not Defteri kısmını birleştir'
    strcat(baslik, " - GTK Not Defteri");

    // Başlığa koy
    gtk_window_set_title(gwindow, baslik);

    // Hafızayı boşalt! Çok Önemli
    free(baslik);
    free(fn);
}


void notepad_hakkinda(void) {
    GtkWidget *dialog = gtk_about_dialog_new();
    GtkAboutDialog *dlg = GTK_ABOUT_DIALOG(dialog);
    //gtk_about_dialog_set_name(dlg, "GTK Notepad");
    gtk_about_dialog_set_version(dlg, "Versiyon: 1");
    gtk_about_dialog_set_copyright(dlg, "(c) İSİM BURAYA GELECEK");
    gtk_about_dialog_set_comments(dlg, "GTK Not Defteri, C ve GTK kullanılarak yazılmış profesyonel bir uygulamadır.");

    // Web sitesi yoksa bu satır silinecek
    gtk_about_dialog_set_website(dlg, "http://www.website.com");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}


int notepad_kaydet_veya_iptal_sor(void) {
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(gwindow,     // Ekran oluştur
                GTK_DIALOG_DESTROY_WITH_PARENT,  // Üst ekran ile kapatılacak şekilde
                GTK_MESSAGE_QUESTION,            // Soru soracak şekilde
                GTK_BUTTONS_YES_NO,              // Evet Hayır cevaplarıyla
                "Dosya değiştirildi, en son yaptığınız değişiklikleri kaydetmek istiyor musunuz?");

    // İptal tuşu ekstra olarak ekleniyorö bu silinebilir
    gtk_dialog_add_button(GTK_DIALOG(dialog), "İptal", GTK_RESPONSE_CANCEL);

    // Başlık
    gtk_window_set_title(GTK_WINDOW(dialog), "Değişiklikleri Kaydet");

    // Cevabı int olarak al
    int response = gtk_dialog_run(GTK_DIALOG(dialog));

    // Buraya geldiysek, cevap verilmiştir. Ekranı hafızadan sil
    gtk_widget_destroy(dialog);

    // Cevabı döndür
    return response;
}


// region Olaylar

// Ekranin ikonunu degistirmek icin
GdkPixbuf *ikon_olustur(const char *filename) {
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    pixbuf = gdk_pixbuf_new_from_file(filename, &error);
    if (!pixbuf) {
        fprintf(stderr, "%s: %s\n",
                PROGNAME,
                error->message);
        g_error_free(error);
    }

    return pixbuf;
}

/**
 * Metin değiştirildiğinde çalışacak
 */
void notepad_metin_degisti(void) {
    degisiklik = TRUE;
    notepad_pozisyon_guncelle();
}

/**
 * Sırasıyla kes, kopyala, yapıştır ve sil
 */
void notepad_kes(void) {
    gtk_text_buffer_cut_clipboard(buffer, clipboard, TRUE);
}

void notepad_kopyala(void) {
    gtk_text_buffer_copy_clipboard(buffer, clipboard);
}

void notepad_yapistir(void) {
    gtk_text_buffer_paste_clipboard(buffer, clipboard, NULL, TRUE);
}

void notepad_sil(void) {
    gtk_text_buffer_delete_selection(buffer, TRUE, TRUE);
}

/**
 * Hepsini seçme fonksiyonu, GtkTextIter aracı, bir metnin başını ve sonunu sayı olarak verebilir
 * ve bununla gtk_text_buffer_select_range ile seçim yapılcak noktaları belirtmiş oluruz
 */
void notepad_hepsini_sec(void) {
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);

    gtk_text_buffer_select_range(buffer, &start, &end);
}

/**
 * !! Önemli, tarih girme işlemi, bilgisyarın time() sonucu verdiği tarihi alır ve asctime, localtime fonksiyonu
 * sonucunu döndürür ve bu sonuç, bilgisayar yerine ve diline bağlı olarak değişebilir
 */
void notepad_tarih_gir(void) {
    time_t now = time(0);
    char* datetime = asctime(localtime(&now));

    gtk_text_buffer_insert_at_cursor(buffer, datetime, strlen(datetime) - 1);
}

/**
 * Cümle kesme değişti, GTK cümle kesmeyi destekler, uzun cümle ekrana sığmadığı zaman
 * nasıl bölüneceğinin seçimini böylelikle yapıp, GtkTextView yazı alanımıza bu özelliği veriyoruz
 *
 * !! Önemli, Olan özelliği değiştirir bu fonksiyon
 */
void notepad_kesmeyi_degistir(void) {
    GtkWrapMode mode;

    // Diğer Seçenekler: GTK_WRAP_WORD,  GTK_WRAP_WORD_CHAR
    // Sırasıyla kelimeyi direkt aşağı yaz, kelime ve karakter olarak böl

    // Eğer cumle_bolumu açık ise kapat, kapalı ise aç
    // Çünkü bu fonksiyon değiştirme fonksiyonu

    if (cumle_bolumu)
        mode = GTK_WRAP_NONE;
    else
        mode = GTK_WRAP_CHAR;

    cumle_bolumu = !cumle_bolumu;
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textarea), mode);
}

/**
 * Yazı tipi seçici, ekran açılır ve sonuç beklenir. Sonuç ardından textarea mıza uygulanır
 */
void notepad_yazi_tipi_sec(void) {
    GtkResponseType result;

    GtkWidget *dialog = gtk_font_selection_dialog_new("Font Seçin");

    result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_OK || result == GTK_RESPONSE_APPLY) {
        PangoFontDescription *font_desc;
        char *font_adi = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));

        font_desc = pango_font_description_from_string(font_adi);

        gtk_widget_modify_font(textarea, font_desc);

        g_free(font_adi);
    }

    gtk_widget_destroy(dialog);
}

/**
 * Caret (Mouse göstergesi) in bulunduğu yeri gösterir ve bu değişiklikleri statusbar'a koyar
 */
void notepad_pozisyon_guncelle(void) {
    char *durum;
    int satir, sutun;

    // Pozisyonları almak için textiter
    GtkTextIter iter;

    // statusbarımız
    GtkStatusbar *gstatusbar = GTK_STATUSBAR(statusbar);

    /**
     * Gtk status bar, stack mantığı ile çalışır (Push / Pop) yani yeni mesaj sıraya alınır, ilk sıradaki
     * çıkarılır gibi bir sıralı sistem uygulanmakta, burada ilk sıradaki mesajı (şuanda gösterilen)
     * siliyor ve altta yeni mesajı push (sıraya alma) ediyoruz
     */
    gtk_statusbar_pop(gstatusbar, 0);

    // Şuanki yerin bilgilerini al
    gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));

    satir = gtk_text_iter_get_line(&iter);
    sutun = gtk_text_iter_get_line_offset(&iter);

    durum = g_strdup_printf("Satır: %d Sütun: %d", satir + 1, sutun + 1);

    gtk_statusbar_push(gstatusbar, 0, durum);

    g_free(durum);
}

// endregion

int main(int argc, char** argv);

void yeni_ekran_ac() {
   /*unsigned int nameLen = strlen(PROGNAME);
    char* path = malloc(nameLen + 3);
    path[0] = '"';
    path[nameLen + 1] = '"';
    path[nameLen + 2] = '\0';
    memcpy(path + 1, PROGNAME, nameLen);
    system(path);*/

    unsigned int startLen = strlen("START \"\" \"");
    unsigned int programNameLength = strlen(PROGNAME);

    char* path = malloc(startLen + programNameLength + 2);
    memcpy(path, "START \"\" \"", startLen);
    memcpy(path + 10, PROGNAME, programNameLength);
    path[startLen + programNameLength] = '"';
    path[startLen + programNameLength + 1] ='\0';
    system(path);
}

/**
 * Bu isimlenirmeler için yukarı bakın
 */
void notepad_yeni_dosya(void) {
    yeni_ekran_ac();
    return;
    free(dosya_adi);
    dosya_adi = malloc(1);
    dosya_adi[0] = '\0';

    gtk_text_buffer_set_text(buffer, "", -1);

    gtk_window_set_title(gwindow, "İsimsiz - GTK Not Defteri");
}

char notepad_dosya_ac(const char *dosya_adi) {
    FILE *dosya = fopen(dosya_adi, "rb");

    // Dosya bulunamadı veya hata esnasında !fp true olacak ve bir alt bloğa gireceğiz, hatayı yazdırıp çıkıyoruz
    if (!dosya) {
        fprintf(stderr, "%s: fopen(null): %s\n",
                PROGNAME,
                strerror(errno));
        fclose(dosya);

        return 0;
    }
    else {
        // Hata yoksa buradayız, klasik c stili dosya okuma işlemleri yapıp metni gtk_text_buffer_set_text ile
        // alıyoruz

        char* string;
        fseek(dosya, 0, SEEK_END);
        long dosya_boyutu = ftell(dosya);
        fseek(dosya, 0, SEEK_SET);

        // !! ÖNEMLİ, burası çok önemli, dosyayı okumak için dosyadaki harf sayısı + 1 yer açtık
        // ve en sonki 1 boşluğuna NULL TERMINATOR denilen, stringin bittiğini ifade eden \0 ekledik
        string = malloc(dosya_boyutu + 1);
        fread(string, dosya_boyutu, 1, dosya);
        string[dosya_boyutu] = '\0';

        gtk_text_buffer_set_text(buffer, string, -1);

        free(string);
        fclose(dosya);

        notepad_basligi_degistir(dosya_adi);

        // En son metin koyulduğunda değişiklik yapılmış olmaz
        degisiklik = FALSE;
    }

    return 1;
}

void notepad_yeni_dosya_ac(void) {
    // Değişiklik varsa
    if (degisiklik == TRUE) {
        int response = notepad_kaydet_veya_iptal_sor();

        // Cevaba göre işlem yapılıyor. Break ve return arasındaki farka dikkat!
        // Breaklar aşağıya inip okumaya devam ederken returnler direkt okumayı bırakıp
        // fonksiyondan çıkıyorlar
        switch (response) {
            case GTK_RESPONSE_YES:
                notepad_kaydet();
                break;
            case GTK_RESPONSE_NO:
                break;

            case GTK_RESPONSE_DELETE_EVENT:
            case GTK_RESPONSE_CANCEL:
                return;

            default:
                return;
        }
    }

    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    int res;

    dialog = gtk_file_chooser_dialog_new("Dosya Seç",
                                         gwindow,
                                         action,
                                         "_Aç",
                                         GTK_RESPONSE_ACCEPT,
                                         "_İptal",
                                         GTK_RESPONSE_CANCEL,
                                         NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    char* filename;
    if (res == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        if (notepad_dosya_ac(filename)) {
            free(dosya_adi);
            dosya_adi = malloc(strlen(filename) + 1);
            strcpy(dosya_adi, filename);
        }

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

char dosyayi_kaydet(const char *dosya_adi) {
    // Buradaki işlemleri anlamak için dosya okuma fonksiyonuna gidin
    // İşlemler birebir aynı, \0 hariç
    FILE *fp = fopen(dosya_adi, "wb");
    if (!fp) {
        fprintf(stderr, "%s: fopen(null): %s\n",
                PROGNAME,
                strerror(errno));
        fclose(fp);
        return 0;
    }
    else {
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(buffer, &start);
        gtk_text_buffer_get_end_iter(buffer, &end);

        char* string = gtk_text_buffer_get_text(buffer,
                                             &start,
                                             &end,
                                             TRUE);

        fwrite(string, strlen(string), 1, fp);
        free(string);
        fclose(fp);

        notepad_basligi_degistir(dosya_adi);

        degisiklik = FALSE;
    }

    return 1;
}

/**
 * Şimdiki dosyayı kaydet
 */
void notepad_kaydet(void) {
    if (DOSYA_ACILMADI) // ise nasıl kaydedeceğini sor
        dosyayi_farkli_kaydet();
    else
        dosyayi_kaydet(dosya_adi); // değilse, bunu aynı yere kaydet
}

void dosyayi_farkli_kaydet(void) {
    GtkWidget *dialog;
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    int res;

    dialog = gtk_file_chooser_dialog_new("Dosyayı Kaydet",
                                         gwindow,
                                         action,
                                         "_Kaydet",
                                         GTK_RESPONSE_ACCEPT,
                                         "_İptal",
                                         GTK_RESPONSE_CANCEL,
                                         NULL);

    chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);

    if (dosya_adi)
        gtk_file_chooser_set_filename(chooser, dosya_adi);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    char* filename;
    if (res == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(
                        GTK_FILE_CHOOSER(dialog));

        if (dosyayi_kaydet(filename)) {
            free(dosya_adi);
            dosya_adi = malloc(strlen(filename) + 1);
            strcpy(dosya_adi, filename);
        }

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

/**
 * Menü çubuğunu oluşturma fonksiyonu, içerik hep aynı şekilde
 */
void menubari_olustur(void) {
    menubar = gtk_menu_bar_new();

    filemenu = gtk_menu_new();
    file = gtk_menu_item_new_with_mnemonic("_Dosya");
    new  = gtk_menu_item_new_with_mnemonic("_Yeni");
    open = gtk_menu_item_new_with_mnemonic("_Aç");
    save = gtk_menu_item_new_with_mnemonic("_Kaydet");
    saveas = gtk_menu_item_new_with_mnemonic("... Olarak _Kaydet");
    quit = gtk_menu_item_new_with_mnemonic("_Çıkış");

    editmenu = gtk_menu_new();
    edit = gtk_menu_item_new_with_mnemonic("_Düzenle");
    cut  = gtk_menu_item_new_with_mnemonic("Ke_s");
    copy = gtk_menu_item_new_with_mnemonic("_Kopyala");
    paste = gtk_menu_item_new_with_mnemonic("_Yapıştır");
    delete = gtk_menu_item_new_with_mnemonic("_Sil");
    selectall = gtk_menu_item_new_with_mnemonic("_Hepsini Seç");
    time_date = gtk_menu_item_new_with_mnemonic("_Zaman/Tarih");
    wll  = gtk_check_menu_item_new_with_mnemonic("_Uzun satırları böl");
    select_font = gtk_menu_item_new_with_mnemonic("_Yazı Tipi...");

    helpmenu = gtk_menu_new();
    help = gtk_menu_item_new_with_mnemonic("_Yardım");
    about = gtk_menu_item_new_with_mnemonic("_Hakkında");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), new);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), open);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), save);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), saveas);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),
                            gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit), editmenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), cut);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), copy);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), paste);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), delete);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),
                            gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), selectall);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), time_date);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),
                            gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), wll);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), select_font);

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), helpmenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), about);

    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), edit);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help);

    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 3);

    // !!Önemli Kısa yollar

    // Dosya menüsü
    gtk_widget_add_accelerator(new,  "activate", accel, GDK_KEY_N,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(open, "activate", accel, GDK_KEY_O,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(save, "activate", accel, GDK_KEY_S,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(saveas, "activate", accel, GDK_KEY_S,
                               GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                               GTK_ACCEL_VISIBLE);

    gtk_widget_add_accelerator(quit, "activate", accel, GDK_KEY_Q,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);

    // Düzenle
    gtk_widget_add_accelerator(cut, "activate", accel, GDK_KEY_X,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(copy, "activate", accel, GDK_KEY_C,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(paste, "activate", accel, GDK_KEY_V,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(delete, "activate", accel, GDK_KEY_Delete,
                               0,
                               GTK_ACCEL_VISIBLE);

    gtk_widget_add_accelerator(selectall, "activate", accel, GDK_KEY_A,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(time_date, "activate", accel, GDK_KEY_F5,
                               0,
                               GTK_ACCEL_VISIBLE);

    // !!Önemli Tıklama Olayları

    // Dosya
    g_signal_connect(new, "activate", (GCallback) notepad_yeni_dosya, NULL);
    g_signal_connect(open, "activate", (GCallback) notepad_yeni_dosya_ac, NULL);
    g_signal_connect(save, "activate", (GCallback) notepad_kaydet, NULL);
    g_signal_connect(saveas, "activate", (GCallback) dosyayi_farkli_kaydet, NULL);
    g_signal_connect(quit, "activate", (GCallback) gtk_main_quit, NULL);

    // Düzenle
    g_signal_connect(cut, "activate", (GCallback) notepad_kes, NULL);
    g_signal_connect(copy, "activate", (GCallback) notepad_kopyala, NULL);
    g_signal_connect(paste, "activate", (GCallback) notepad_yapistir, NULL);
    g_signal_connect(delete, "activate", (GCallback) notepad_sil, NULL);
    g_signal_connect(selectall, "activate", (GCallback) notepad_hepsini_sec, NULL);
    g_signal_connect(time_date, "activate", (GCallback) notepad_tarih_gir, NULL);
    g_signal_connect(wll, "activate", (GCallback) notepad_kesmeyi_degistir, NULL);
    g_signal_connect(select_font, "activate", (GCallback) notepad_yazi_tipi_sec, NULL);

    // Yardım
    g_signal_connect(about, "activate", (GCallback) notepad_hakkinda, NULL);
}


void yazi_alanini_olustur(void) {
    kaydirilabilir_ekran = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(
                                   kaydirilabilir_ekran),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);

    textarea = gtk_text_view_new();

    gtk_container_add(GTK_CONTAINER(kaydirilabilir_ekran), textarea);

    gtk_box_pack_start(GTK_BOX(vbox), kaydirilabilir_ekran, TRUE, TRUE, 0);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textarea));

    // Olaylar, metin değişti vs vs
    g_signal_connect(buffer, "changed", (GCallback) notepad_metin_degisti, NULL);
    g_signal_connect(buffer, "mark_set", (GCallback) notepad_pozisyon_guncelle, NULL);

    notepad_kesmeyi_degistir();
}


void durum_cubugu_olustur(void) {
    statusbar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, FALSE, 0);

    notepad_pozisyon_guncelle();
}


int main(int argc, char* argv[]) {
    PROGNAME = argv[0];

    /**
     * Hata olması durumunda dosyayı açarken, tekrar, dosya_adi pointer hatasi almamak için
     * ilk olarak onu bir dolduruyoruz
     */
    dosya_adi = malloc(1);
    dosya_adi[0] = '\0';

    // Dosya okuma işlemleri için yukarıya bakın
    if (argc == 2) {
        FILE *temp = fopen(argv[1], "rb");
        if (!temp) {
            fprintf(stderr, "%s: failed to open file: %s\n",
                    PROGNAME,
                    strerror(errno));
            return EXIT_FAILURE;
        }

        fclose(temp);

        free(dosya_adi);
        dosya_adi = malloc(strlen(argv[1]));
        strcpy(dosya_adi, argv[1]);
    }
    else if (argc > 2) {
        kullanimi_yazdir();
        return EXIT_FAILURE;
    }

    gtk_init(&argc, &argv);

    // Kopyalama Panosunu al
    atom = gdk_atom_intern("CLIPBOARD", TRUE);
    clipboard = gtk_clipboard_get(atom);

    // Ekran
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gwindow = GTK_WINDOW(window);

    // Ekran seçenekleri
    gtk_window_set_title(gwindow, program_basligi);
    gtk_window_set_default_size(gwindow, 640, 480);
    gtk_window_set_position(gwindow, GTK_WIN_POS_CENTER);

    /**
     * Programda ikon değişikliği yapmak için aşağıdaki ifadedeki yorumu silin
     * ikon oluşacaktır. İkon bilgilerine gtk-notepad.h dosyasından erişin
     */
    // gtk_window_set_icon(gwindow, ikon_olustur(ikon));

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    accel = gtk_accel_group_new();
    gtk_window_add_accel_group(gwindow, accel);

    menubari_olustur();
    yazi_alanini_olustur();
    durum_cubugu_olustur();

    if (argc == 2)
        notepad_dosya_ac(dosya_adi);

    // Çıkış olayları
    g_signal_connect(window, "destroy", (GCallback) gtk_main_quit, NULL);

    // Hepsini göster ve başla
    gtk_widget_show_all(window);
    gtk_main();

    return EXIT_SUCCESS;
}