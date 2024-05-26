#include <gtk/gtk.h>
#include <vte/vte.h>
#include <pango/pango.h>
#include <stdlib.h>

GtkWindow *window;
GtkWidget *term;
GdkRGBA palette[16];
PangoFontDescription *font;
double font_scale = 1;
double resize_scale = 1.1;

char *cmd[16] = { "/bin/bash", NULL };
const char *default_font = "ShureTechMono Nerd Font 15";
const char *colors[16] = {
    "#101010", "#ec6c85", "#666666", "#AAAAAA",
    "#ec6c85", "#AAAAAA", "#666666", "#EEEEEE",
    "#101010", "#ec6c85", "#666666", "#AAAAAA",
    "#ec6c85", "#AAAAAA", "#666666", "#EEEEEE",
};
#define TERM_KEY(k) (event->keyval == (k) && modifiers == (GDK_CONTROL_MASK|GDK_SHIFT_MASK))


static void
set_font_scale(double scale)
{
    font_scale = scale;
    vte_terminal_set_font_scale(VTE_TERMINAL(term), font_scale);
}

static void
term_quit(VteTerminal *term, int status, gpointer user_data)
{
    gtk_window_close(window);
}

static void
term_title(VteTerminal *term, gpointer user_data)
{
    const char *title;
    title = vte_terminal_get_window_title(term);
    gtk_window_set_title(window, title);
}

static void
setup_terminal(VteTerminal *term)
{
    font = pango_font_description_from_string(default_font);
    for (int i = 0; i < 16; ++i) {
        gdk_rgba_parse(palette + i, colors[i]);
    }

    vte_terminal_spawn_async(
        term, VTE_PTY_DEFAULT,
        NULL, cmd, NULL,
        G_SPAWN_DEFAULT,
        NULL, NULL, NULL,
        -1,
        NULL, NULL, NULL);

    vte_terminal_set_cursor_blink_mode(term, VTE_CURSOR_BLINK_OFF);
    vte_terminal_set_colors(term, &palette[15], &palette[0], palette, 16);
    vte_terminal_set_font(term, font);
    vte_terminal_set_bold_is_bright(term, TRUE);
    vte_terminal_set_font_scale(term, font_scale);
    vte_terminal_set_enable_bidi(term, FALSE);

    g_signal_connect(term, "window-title-changed", G_CALLBACK(term_title), NULL);
    g_signal_connect(term, "child-exited", G_CALLBACK(term_quit), NULL);
}

static gboolean
key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    GdkModifierType modifiers;
    modifiers = event->state & gtk_accelerator_get_default_mod_mask();

    if (TERM_KEY(GDK_KEY_C)) {
        vte_terminal_copy_clipboard_format(VTE_TERMINAL(term), VTE_FORMAT_TEXT);
    }
    else if (TERM_KEY(GDK_KEY_V)) {
        vte_terminal_paste_clipboard(VTE_TERMINAL(term));
    }
    else if (TERM_KEY(GDK_KEY_plus)) {
        set_font_scale(font_scale * resize_scale);
    }
    else if (TERM_KEY(GDK_KEY_underscore)) {
        set_font_scale(font_scale / resize_scale);
    }
    else if (TERM_KEY(GDK_KEY_BackSpace)) {
        set_font_scale(1);
    }
    else {
        return FALSE;
    }
    return TRUE;
}

int
main(int argc, char **argv)
{
    gtk_init(&argc, &argv);
    // TODO: check boundaries... or nah
    if (argc > 1) {
        cmd[1] = "-c";
        for (int i = 1; i < argc; ++i) {
            cmd[i+1] = argv[i];
        }
    }

    window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    gtk_window_set_default_size(window, 900, 540);
    gtk_window_set_title(window, "miuterm");

    g_signal_connect(GTK_WIDGET(window), "key-press-event", G_CALLBACK(key_press), NULL);
    g_signal_connect(GTK_WIDGET(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    term = vte_terminal_new();
    gtk_container_add(GTK_CONTAINER(window), term);

    setup_terminal(VTE_TERMINAL(term));

    gtk_widget_show_all(GTK_WIDGET(window));
    gtk_widget_grab_focus(term);

    gtk_main();
    return 0;
}
