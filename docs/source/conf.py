# -- Project information -----------------------------------------------------
project = "patternia"
copyright = "2025, sentomk"
author = "sentomk"
release = "0.4.2"

# -- General configuration ---------------------------------------------------
extensions = [
    "myst_parser",
    "breathe",
    "sphinx.ext.autodoc",
    "sphinx.ext.todo",
    "sphinx.ext.githubpages",
    "sphinx.ext.viewcode",
    "sphinx.ext.autosectionlabel",
]

templates_path = ["_templates"]
exclude_patterns = []

source_suffix = {
    ".rst": "restructuredtext",
    ".md": "markdown",
}

# -- Options for Breathe -----------------------------------------------------
breathe_projects = {
    "patternia": "../build/doxygen/xml"
}
breathe_default_project = "patternia"
breathe_default_members = ("members", "undoc-members")
breathe_show_define_initializer = True
breathe_show_enumvalue_initializer = True

pygments_style = "friendly"
pygments_dark_style = "monokai"

# -- Options for HTML output -------------------------------------------------
html_theme = "furo"
html_static_path = ["_static"]
html_title = "patternia"
html_favicon = "_static/favicon.ico"

# Light / Dark logo
html_theme_options = {
    "light_logo": "logo-light.png",
    "dark_logo": "logo-dark.png",
    "sidebar_hide_name": True,
    "navigation_with_keys": True,
    "light_css_variables": {
        "color-brand-primary": "#2d74da",
        "color-brand-content": "#007acc",
    },
    "dark_css_variables": {
        "color-brand-primary": "#8ab4f8",
        "color-brand-content": "#80bfff",
    },
}

html_css_files = ["custom.css"]

# -- Miscellaneous tweaks ----------------------------------------------------
highlight_language = "cpp"
pygments_style = "sphinx"
todo_include_todos = True
