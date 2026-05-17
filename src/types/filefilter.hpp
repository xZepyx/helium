#pragma once

#include <gtk/gtk.h>

#include <string>
#include <vector>

class FileFilter {
private:
    GtkFileFilter* _filter;

public:
    FileFilter(const std::string& name = "")
        : _filter(gtk_file_filter_new())
    {
        if (!name.empty()) {
            gtk_file_filter_set_name(_filter, name.c_str());
        }
    }

    void set_name(const std::string& name) {
        gtk_file_filter_set_name(_filter, name.c_str());
    }

    std::string get_name() {
        return gtk_file_filter_get_name(_filter);
    }

    void add_pattern(const std::string& pattern) {
        gtk_file_filter_add_pattern(_filter, pattern.c_str());
    }

    void add_mime_type(const std::string& mime) {
        gtk_file_filter_add_mime_type(_filter, mime.c_str());
    }

    void add_suffix(const std::string& suffix) {
        gtk_file_filter_add_suffix(_filter, suffix.c_str());
    }

    GtkFileFilter* get_native() { return _filter; }

    static FileFilter* for_images() {
        auto f = new FileFilter("Images");
        f->add_mime_type("image/png");
        f->add_mime_type("image/jpeg");
        f->add_mime_type("image/gif");
        f->add_mime_type("image/svg+xml");
        f->add_mime_type("image/webp");
        return f;
    }

    static FileFilter* for_audio() {
        auto f = new FileFilter("Audio");
        f->add_mime_type("audio/mpeg");
        f->add_mime_type("audio/ogg");
        f->add_mime_type("audio/wav");
        f->add_mime_type("audio/flac");
        return f;
    }

    static FileFilter* for_video() {
        auto f = new FileFilter("Video");
        f->add_mime_type("video/mp4");
        f->add_mime_type("video/webm");
        f->add_mime_type("video/ogg");
        return f;
    }

    static FileFilter* for_all() {
        auto f = new FileFilter("All Files");
        f->add_pattern("*");
        return f;
    }
};
