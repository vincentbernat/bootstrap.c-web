# The web stuff is handled by grunt and not by autotools. Notably, it
# has its own dist and build directory and we only rebuild stuff if we
# have grunt.

EXTRA_DIST = \
	package.json \
	Gruntfile.js \
	nginx.conf \
	app/index.html \
	app/images \
	app/styles \
	app/scripts \
	app/components \
	dist

WEBDIST=$(srcdir)/dist

# Install web files
all-local: $(WEBDIST)
install-data-hook: $(WEBDIST)
	$(MKDIR_P) $(DESTDIR)$(pkgdatadir)/web
	$(AM_V_at)cp -fpRT $(WEBDIST) $(DESTDIR)$(pkgdatadir)/web
uninstall-hook:
	$(AM_V_at)rm -rf $(DESTDIR)$(pkgdatadir)/web

# Rebuild with grunt if available
if HAVE_GRUNT
$(WEBDIST): $(srcdir)/app
	$(AM_V_GEN)cd $(srcdir) && $(GRUNT) dist
clean-local:
	cd $(srcdir) && $(GRUNT) clean
else
dist: $(srcdir)/app
	@echo " *** grunt is not present, reuse the dist directory as is"
	[ -d $(WEBDIST) ]
endif
