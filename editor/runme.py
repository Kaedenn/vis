#!/usr/bin/env python3

"""
Custom local HTTP server that disables caching.

This is to ensure changes to files being served are properly delivered
to the web browser.
"""

import argparse
from http.server import HTTPServer, SimpleHTTPRequestHandler
import logging
import os

logging.basicConfig(format="%(module)s:%(lineno)s: %(levelname)s: %(message)s",
                    level=logging.INFO)
logger = logging.getLogger(__name__)

class NoCacheHandler(SimpleHTTPRequestHandler):
  "Custom handler that disables caching"
  def end_headers(self):
    "Add the 'disable cache' headers"
    self.send_header("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0")
    self.send_header("Pragma", "no-cache")
    self.send_header("Expires", "0")
    super().end_headers()

def main():
  ap = argparse.ArgumentParser()
  ap.add_argument("port", type=int, default=8000, nargs="?",
      help="listen on port %(metavar)s (default: %(default)s)")
  ap.add_argument("-b", "--bind", default="localhost",
      help="bind to hostname %(metavar)s (default: %(default)r)")
  ap.add_argument("-d", "--directory", metavar="DIR",
      help="serve files from %(metavar)s (default: current directory)")
  ap.add_argument("-n", "--no-cache", action="store_true",
      help="disable caching, which can be problematic at times")
  args = ap.parse_args()

  dirname = "." if args.directory is None else args.directory
  logger.info("Serving files from %s on http://%s:%d/",
      dirname, args.bind, args.port)
  if args.directory is not None:
    os.chdir(args.directory)
  if args.no_cache:
    server = HTTPServer((args.bind, args.port), NoCacheHandler)
  else:
    server = HTTPServer((args.bind, args.port), SimpleHTTPRequestHandler)
  server.serve_forever()

if __name__ == "__main__":
  try:
    main()
  except KeyboardInterrupt:
    raise SystemExit(1)

# vim: set ts=2 sts=2 sw=2:
