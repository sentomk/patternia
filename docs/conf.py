project = 'Patternia'
author = 'SentoMK'
release = '0.4.2'

extensions = [
    'breathe',
    'sphinx.ext.autodoc',
    'sphinx.ext.viewcode'
]

breathe_projects = { "Patternia": "./xml" }
breathe_default_project = "Patternia"

html_theme = 'furo'
