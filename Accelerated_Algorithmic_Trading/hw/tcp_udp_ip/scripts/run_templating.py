# Run templating for the RTL outputs

import argparse
import os.path
import yaml
import sys
from jinja2 import Environment
from jinja2.loaders import FileSystemLoader


parser = argparse.ArgumentParser(description='Generate templated outputs')
parser.add_argument("template_file")
parser.add_argument("context_file", help="context file in yaml format")
parser.add_argument("-o", "--output", help="output file (defaults to stdout)")
parser.add_argument("--template_dir", help="template directory (defaults to script directory)")

args = parser.parse_args()

template_file = args.template_file
context_file  = args.context_file

if args.template_dir:
    template_dir = args.template_dir
else:
    template_dir = os.path.dirname(os.path.abspath(__file__))

# Read in the context
with open(context_file, 'r') as f:
    context = yaml.load(f, Loader=yaml.FullLoader)

environment = Environment(loader=FileSystemLoader(template_dir),
    line_statement_prefix = '//%',
    line_comment_prefix   = '///')

rtl = environment.get_template(os.path.basename(template_file))

if args.output:
    with open(args.output, 'w') as f:
        f.write(rtl.render(context))
else:
    f = sys.stdout
    f.write(rtl.render(context))

