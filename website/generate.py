#!/usr/bin/env python3
"""Convert docs/ markdown files to website HTML pages."""
import os, re, html as html_mod

DOCS = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "docs"))
OUT = os.path.abspath(os.path.join(os.path.dirname(__file__)))

SIDEBAR = []

def esc(text):
    return html_mod.escape(text)

def parse_md(text):
    """Convert simple markdown to HTML."""
    lines = text.split("\n")
    out = []
    i = 0
    in_code = False
    code_buf = []
    code_lang = ""
    in_table = False
    table_buf = []

    while i < len(lines):
        line = lines[i]

        # Code block
        if line.startswith("```"):
            if in_code:
                code = "\n".join(code_buf)
                out.append(f'<div class="code-block-wrap"><pre><code class="language-{code_lang}">{esc(code)}</code></pre></div>\n')
                code_buf = []
                code_lang = ""
                in_code = False
                i += 1
                continue
            else:
                in_code = True
                code_lang = line[3:].strip()
                i += 1
                continue

        if in_code:
            code_buf.append(line)
            i += 1
            continue

        # Skip empty lines sometimes
        # Tables
        if "|" in line and line.strip().startswith("|"):
            in_table = True
            table_buf.append(line)
            i += 1
            continue
        elif in_table and (not line.strip() or not "|" in line):
            # flush table
            out.append(parse_table(table_buf))
            table_buf = []
            in_table = False
            # continue processing current line
            if not line.strip():
                i += 1
                continue
        elif in_table:
            in_table = False
            out.append(parse_table(table_buf))
            table_buf = []

        if not line.strip():
            out.append("\n")
            i += 1
            continue

        # Headings
        m = re.match(r'^(#{1,3})\s+(.+)$', line)
        if m:
            level = len(m.group(1))
            text = m.group(2)
            anchor = re.sub(r'[^a-zA-Z0-9]+', '-', text.lower()).strip('-')
            out.append(f'<h{level} id="{anchor}">{parse_inline(text)}</h{level}>\n')
            i += 1
            continue

        # Unordered list
        if re.match(r'^[-*+]\s+', line):
            out.append(f'<li>{parse_inline(re.sub(r"^[-*+]\s+", "", line))}</li>\n')
            i += 1
            continue

        # Ordered list
        m = re.match(r'^\d+\.\s+(.+)$', line)
        if m:
            out.append(f'<li>{parse_inline(m.group(1))}</li>\n')
            i += 1
            continue

        # Paragraph
        out.append(f'<p>{parse_inline(line)}</p>\n')
        i += 1

    # Flush table if still open
    if in_table and table_buf:
        out.append(parse_table(table_buf))

    # Wrap consecutive <li> in <ul>
    result = []
    in_ul = False
    for line in out:
        if line.startswith('<li>'):
            if not in_ul:
                result.append('<ul>\n')
                in_ul = True
            result.append(line)
        else:
            if in_ul:
                result.append('</ul>\n')
                in_ul = False
            result.append(line)
    if in_ul:
        result.append('</ul>\n')

    return ''.join(result)

def parse_table(lines):
    """Convert markdown table to HTML."""
    rows = []
    for line in lines:
        line = line.strip()
        if not line or line.startswith("|---") or line.startswith("|:---"):
            continue
        cells = [c.strip() for c in line.strip("|").split("|")]
        rows.append(cells)

    html = '<table>\n'
    for idx, row in enumerate(rows):
        tag = 'th' if idx == 0 else 'td'
        html += '<tr>\n'
        for cell in row:
            html += f'  <{tag}>{parse_inline(cell)}</{tag}>\n'
        html += '</tr>\n'
    html += '</table>\n'
    return html

def parse_inline(text):
    """Bold, code, links."""
    # Bold: **text** or __text__
    text = re.sub(r'\*\*(.+?)\*\*', r'<strong>\1</strong>', text)
    text = re.sub(r'__(.+?)__', r'<strong>\1</strong>', text)
    # Inline code
    text = re.sub(r'`([^`]+)`', r'<code>\1</code>', text)
    # Links
    text = re.sub(r'\[([^\]]+)\]\(([^)]+)\)', r'<a href="\2">\1</a>', text)
    return text

def make_slug(name):
    return name.lower().replace(" ", "-")

def collect_docs():
    """Build list of all doc files."""
    docs = []
    root = DOCS
    for dirpath, dirnames, filenames in os.walk(root):
        for fn in sorted(filenames):
            if not fn.endswith(".md"):
                continue
            full = os.path.join(dirpath, fn)
            rel = os.path.relpath(full, root)
            docs.append(rel)
    docs.sort()
    return docs

def build_sidebar(current=None):
    """Build sidebar HTML."""
    sections = {
        "Getting Started": ["introduction.md", "compositor.md", "managers.md"],
        "Types": sorted([f"types/{f}" for f in os.listdir(os.path.join(DOCS, "types")) if f.endswith(".md")]),
        "Services": sorted([f"services/{f}" for f in os.listdir(os.path.join(DOCS, "services")) if f.endswith(".md")]),
    }

    cur_dir = os.path.dirname(current) + "/" if current and "/" in current else ""

    def doc_title(relpath):
        name = os.path.splitext(os.path.basename(relpath))[0]
        return name.replace("-", " ").title()

    def sidebar_url(item):
        url = item.replace(".md", ".html")
        if not cur_dir:
            return url
        if "/" in url:
            if url.startswith(cur_dir):
                return url[len(cur_dir):]
            return "../" + url
        return "../" + url

    html = ""
    for section, items in sections.items():
        html += f'<div class="sidebar-section"><div class="heading">{section}</div>\n'
        for item in items:
            title = doc_title(item)
            url = sidebar_url(item)
            active = ' active' if item == current else ''
            html += f'<a href="{url}" class="nav-link{active}">{title}</a>\n'
        html += '</div>\n'
    return html

def make_page(title, content, current=None, tab="docs"):
    """Wrap content in full page layout."""
    is_docs = current is not None
    css_path = "../style.css" if current and "/" in current else "style.css"
    prefix = "../" if current and "/" in current else ""

    copy_script = '''
document.querySelectorAll('.code-block-wrap').forEach(function(wrap) {
    var btn = document.createElement('button');
    btn.className = 'copy-btn';
    btn.textContent = 'Copy';
    btn.addEventListener('click', function() {
        var code = wrap.querySelector('code');
        navigator.clipboard.writeText(code.textContent).then(function() {
            btn.textContent = 'Copied!';
            btn.classList.add('copied');
            setTimeout(function() { btn.textContent = 'Copy'; btn.classList.remove('copied'); }, 2000);
        });
    });
    wrap.appendChild(btn);
});
'''

    if is_docs:
        sidebar = build_sidebar(current)
        sidebar_html = f'<aside class="sidebar">\n{sidebar}</aside>\n'
        toggle_btn = '<button class="sidebar-toggle" onclick="document.querySelector(\'.sidebar\').classList.toggle(\'open\')">☰</button>'
        wrapper_cls = 'wrapper'
        sidebar_script = '''
document.querySelectorAll('.nav-link').forEach(function(el) {
    el.addEventListener('click', function() {
        document.querySelectorAll('.nav-link').forEach(function(l) { l.classList.remove('active'); });
        this.classList.add('active');
    });
});
'''
    else:
        sidebar_html = ''
        toggle_btn = ''
        wrapper_cls = 'wrapper wrapper-center'
        sidebar_script = ''

    theme_script = '''
var saved = localStorage.getItem('theme') || 'dark';
document.querySelectorAll('.theme-dot').forEach(function(dot) {
    dot.addEventListener('click', function() {
        var theme = this.dataset.theme;
        document.documentElement.className = 'theme-' + theme;
        localStorage.setItem('theme', theme);
        document.querySelectorAll('.theme-dot').forEach(function(d) { d.classList.remove('active'); });
        this.classList.add('active');
    });
    if (dot.dataset.theme === saved) dot.classList.add('active');
});
'''

    script = sidebar_script + copy_script + theme_script

    active_home = ' active' if tab == "home" else ''
    active_docs = ' active' if tab == "docs" else ''
    active_about = ' active' if tab == "about" else ''

    return f'''<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{esc(title)} — helium</title>
<script>(function(){{var t=localStorage.getItem('theme')||'dark';document.documentElement.className='theme-'+t;}})();</script>
<link rel="stylesheet" href="{css_path}">
</head>
<body>

<nav class="topbar">
<div class="topbar-left">
{toggle_btn}
<a href="{prefix}index.html" class="logo">Helium</a>
</div>
<div class="topbar-center">
<div class="nav-links">
<a href="{prefix}index.html"{active_home}>Home</a>
<a href="{prefix}docs.html"{active_docs}>Docs</a>
<a href="{prefix}about.html"{active_about}>About</a>
<a href="https://github.com/xZepyx/helium" class="nav-gh" target="_blank">GitHub</a>
</div>
</div>
<div class="topbar-right">
<div class="theme-dots">
<span class="theme-dot" data-theme="dark" style="background:#7e9cd8" title="Dark"></span>
<span class="theme-dot" data-theme="cream" style="background:#a2b29f" title="Cream"></span>
<span class="theme-dot" data-theme="forest" style="background:#609966" title="Forest"></span>
</div>
</div>
</nav>

<div class="{wrapper_cls}">
{sidebar_html}<main class="content">
{content}
</main>
</div>

<footer class="footer">
helium &middot; built with python and gtk4
</footer>

<script>
{script}
</script>

</body>
</html>'''

def widget_image(name, folder="types", prefix=""):
    path = os.path.join(OUT, "assets", folder, f"{name}.png")
    if os.path.exists(path):
        return f'<img src="{prefix}assets/{folder}/{name}.png" alt="{name}" class="widget-preview">\n'
    return ""

def generate_type_pages():
    types_dir = os.path.join(DOCS, "types")
    out_dir = os.path.join(OUT, "types")
    os.makedirs(out_dir, exist_ok=True)

    for fn in sorted(os.listdir(types_dir)):
        if not fn.endswith(".md"):
            continue
        rel = f"types/{fn}"
        with open(os.path.join(types_dir, fn)) as f:
            raw = f.read()
        name = os.path.splitext(fn)[0]
        title = name.replace("-", " ").title()
        img = widget_image(name, "types", prefix="../")
        body = parse_md(raw)
        page = make_page(f"types/{title}", img + body, current=rel)
        out_fn = fn.replace(".md", ".html")
        with open(os.path.join(out_dir, out_fn), "w") as f:
            f.write(page)
        print(f"  {rel} → types/{out_fn}")

def generate_service_pages():
    svc_dir = os.path.join(DOCS, "services")
    out_dir = os.path.join(OUT, "services")
    os.makedirs(out_dir, exist_ok=True)

    for fn in sorted(os.listdir(svc_dir)):
        if not fn.endswith(".md"):
            continue
        rel = f"services/{fn}"
        with open(os.path.join(svc_dir, fn)) as f:
            raw = f.read()
        name = os.path.splitext(fn)[0]
        title = name.replace("-", " ").title()
        img = widget_image(name, "services", prefix="../")
        body = parse_md(raw)
        page = make_page(f"services/{title}", img + body, current=rel)
        out_fn = fn.replace(".md", ".html")
        with open(os.path.join(out_dir, out_fn), "w") as f:
            f.write(page)
        print(f"  {rel} → services/{out_fn}")

def generate_other_pages():
    others = ["introduction.md", "compositor.md", "managers.md"]
    for fn in others:
        path = os.path.join(DOCS, fn)
        if not os.path.exists(path):
            continue
        with open(path) as f:
            raw = f.read()
        title = os.path.splitext(fn)[0].replace("-", " ").title()
        body = parse_md(raw)
        page = make_page(title, body, current=fn)
        out_fn = fn.replace(".md", ".html")
        with open(os.path.join(OUT, out_fn), "w") as f:
            f.write(page)
        print(f"  {fn} → {out_fn}")

def generate_docs_index():
    """Generate the docs index page."""
    html = '''<h1>Documentation</h1>
<p>All documentation pages organized by category.</p>
'''
    # Types
    html += '<h2>Types</h2>\n<div class="cards">\n'
    for fn in sorted(os.listdir(os.path.join(DOCS, "types"))):
        if not fn.endswith(".md"):
            continue
        name = os.path.splitext(fn)[0]
        title = name.replace("-", " ").title()
        html += f'<a href="types/{name}.html" class="card" style="text-decoration:none"><h3>{title}</h3><p>docs/types/{name}.md</p></a>\n'
    html += '</div>\n'

    # Services
    html += '<h2>Services</h2>\n<div class="cards">\n'
    for fn in sorted(os.listdir(os.path.join(DOCS, "services"))):
        if not fn.endswith(".md"):
            continue
        name = os.path.splitext(fn)[0]
        title = name.replace("-", " ").title()
        html += f'<a href="services/{name}.html" class="card" style="text-decoration:none"><h3>{title}</h3><p>docs/services/{name}.md</p></a>\n'
    html += '</div>\n'

    # Other
    html += '<h2>Guides & Reference</h2>\n<div class="cards">\n'
    for fn in ["introduction.html", "compositor.html", "managers.html"]:
        name = fn.replace(".html", "")
        title = name.replace("-", " ").title()
        html += f'<a href="{fn}" class="card" style="text-decoration:none"><h3>{title}</h3><p>docs/{name}.md</p></a>\n'
    html += '</div>\n'

    page = make_page("Documentation", html, current="introduction.md")
    with open(os.path.join(OUT, "docs.html"), "w") as f:
        f.write(page)
    print("  docs index → docs.html")

def generate_landing():
    html = '''
<div class="hero">
<h1 class="brand"><span class="hl">Helium</span></h1>
<p class="subtitle">Build desktop shell widgets with <span class="hl">Python</span> and <span class="hl">GTK4</span></p>
<div class="actions">
<a href="docs.html" class="btn btn-primary">Read the docs</a>
<a href="https://github.com/xZepyx/helium" class="btn btn-outline">GitHub</a>
</div>

<h2>What you can build</h2>
<div class="cards">
<div class="card">
<span class="icon">⬡</span>
<h3>Bars & Panels</h3>
<p>Top bars, bottom panels, side docks — all with layer-shell exclusivity so nothing overlaps.</p>
</div>
<div class="card">
<span class="icon">◉</span>
<h3>Widgets</h3>
<p>Labels, buttons, sliders, switches, dropdowns, calendars, and 30+ more GTK widgets.</p>
</div>
<div class="card">
<span class="icon">◎</span>
<h3>System Services</h3>
<p>Audio, network, notifications, MPRIS media players, Bluetooth, backlight, wallpapers, and more.</p>
</div>
<div class="card">
<span class="icon">◈</span>
<h3>Compositor IPC</h3>
<p>Built-in Hyprland integration — workspace switching, window tracking, event listening.</p>
</div>
</div>

<h2>Quick start</h2>
<div class="code-block-wrap"><pre><code>import helium
from helium.types import Window, Label

helium.init()

win = Window(
    namespace="hello",
    anchor=["left", "top", "right"],
    exclusivity="exclusive",
)
win.set_child(Label("hello world"))
win.show()

helium.run()</code></pre></div>
<p style="color:var(--on-surface-dim)">Save as <code>bar.py</code> and run with <code>PYTHONPATH=builddir python3 bar.py</code>.</p>

<h2>Project structure</h2>
<div class="feature-grid">
<a href="introduction.html">Introduction & Tutorial</a>
<a href="compositor.html">Hyprland IPC</a>
<a href="managers.html">WindowManager / CSS</a>
<a href="docs.html">All Types (37)</a>
<a href="docs.html">All Services (11)</a>
</div>
</div>'''
    page = make_page("Helium", html, tab="home")
    with open(os.path.join(OUT, "index.html"), "w") as f:
        f.write(page)
    print("  landing → index.html")

def generate_about():
    html = '''
<div class="about-profile">
<img src="assets/pfp.jpg" alt="Profile picture" class="pfp">
<p class="bio">Hey! I'm the developer of helium. Find more about me on <a href="https://github.com/xZepyx">GitHub</a>.</p>
</div>

<div class="about-card">
<div class="row">
<div class="label">GitHub</div>
<div class="value"><a href="https://github.com/xZepyx" target="_blank">@xZepyx</a></div>
</div>
<div class="row">
<div class="label">Reddit</div>
<div class="value">u/ArchPowerUser</div>
</div>
<div class="row">
<div class="label">Discord</div>
<div class="value">@zepyx</div>
</div>
<div class="row">
<div class="label">Email</div>
<div class="value">zepyxunderscore@gmail.com</div>
</div>
<div class="row">
<div class="label">X / Twitter</div>
<div class="value">@ZepyxUnderscore</div>
</div>
</div>

'''
    page = make_page("About", html, tab="about")
    with open(os.path.join(OUT, "about.html"), "w") as f:
        f.write(page)
    print("  about → about.html")

if __name__ == "__main__":
    print("Generating website pages...")
    generate_type_pages()
    generate_service_pages()
    generate_other_pages()
    generate_docs_index()
    generate_landing()
    generate_about()
    print(f"\nDone. Pages written to {OUT}")
