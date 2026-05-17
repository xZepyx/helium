#pragma once

#include <gtk/gtk.h>

#include <functional>
#include <string>

#include "widget.hpp"

class Calendar : public Widget {
public:
    Calendar()
        : Widget(gtk_calendar_new())
    {}

    void select_day(guint year, guint month, guint day) {
        auto date = g_date_time_new_local(year, month, day, 0, 0, 0);
        if (date) {
            gtk_calendar_set_date(GTK_CALENDAR(native), date);
            g_date_time_unref(date);
        }
    }

    void select_month(guint year, guint month) {
        (void)year;
        gtk_calendar_set_month(GTK_CALENDAR(native), month);
    }

    void mark_day(guint day) {
        gtk_calendar_mark_day(GTK_CALENDAR(native), day);
    }

    void unmark_day(guint day) {
        gtk_calendar_unmark_day(GTK_CALENDAR(native), day);
    }

    void clear_marks() {
        gtk_calendar_clear_marks(GTK_CALENDAR(native));
    }

    void set_show_week_numbers(bool show) {
        gtk_calendar_set_show_week_numbers(GTK_CALENDAR(native), show);
    }

    void set_show_heading(bool show) {
        gtk_calendar_set_show_heading(GTK_CALENDAR(native), show);
    }

    void set_show_day_names(bool show) {
        gtk_calendar_set_show_day_names(GTK_CALENDAR(native), show);
    }

    void on_day_selected(std::function<void()> callback) {
        connect_signal("day-selected", callback);
    }

    void on_month_changed(std::function<void()> callback) {
        connect_signal("month-changed", callback);
    }
};
