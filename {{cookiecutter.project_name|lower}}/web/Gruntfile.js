/*jshint globalstrict: true*/
'use strict';

var PORTS = {
  api: 9002,
  connect: 9001,
  web: 9000
};

module.exports = function(grunt) {

  require('load-grunt-tasks')(grunt);
  require('time-grunt')(grunt);

  var path = require('path');

  grunt.initConfig({

    // Watching changes
    watch: {
      html: {
        files: [ 'app/*.html' ],
        tasks: [ 'build:html' ]
      },
      templates: {
        files: [ 'app/templates/*.html' ],
        tasks: [ 'build:templates' ]
      },
      styles: {
        files: [ 'app/styles/*.less' ],
        tasks: [ 'build:styles' ]
      },
      images: {
        files: [ 'app/images/*' ],
        tasks: [ 'build:images' ]
      },
      scripts: {
        files: [ 'app/scripts/{,*/}*.js' ],
        tasks: [ 'build:scripts' ]
      },
      livereload: {
        options: {
          livereload: '<%= connect.options.livereload %>'
        },
        files: [
          'build/*.html',
          'build/styles/*.css',
          'build/images/*',
          'build/scripts/{,*/}*.js' // Including templates
        ]
      }
    },

    // Run the API
    // TODO:3500 To avoid to run the daemon manually when developping, we run it
    // TODO:3500 and access it through nginx. You may want to adapt some
    // TODO:3500 parameters here.
    external_daemon: {
      api: {
        options: {
          startCheck: function(stdout, stderr) { return /listening on /.test(stderr); }
        },
        cmd: 'libtool',
        args: [ 'execute', '../build/src/{{cookiecutter.project_name|lower}}', '-dd',
                '-w', 'build',
                '-l', 'localhost:' + PORTS.api ]
      },
      nginx: {
        cmd: 'nginx',
        args: [ '-c', path.resolve('.', 'nginx.conf') ]
      }
    },

    // Serving app
    connect: {
      options: {
        port: PORTS.connect,
        hostname: '0.0.0.0',
        livereload: 35729
      },
      livereload: {
        options: {
          open: false,
          base: [ 'build' ]
        }
      },
    },

    // Cleaning
    clean: {
      dist: {
        files: [{
          dot: true,
          src: [ 'dist/*', '.tmp' ]
        }]
      },
      build: {
        files: [{
          dot: true,
          src: [ 'build/*' ]
        }]
      }
    },

    // lint
    jshint: {
      options: {
        indent: 2,
        reporter: require('jshint-stylish'),
        globals: {
          // generic
          module: false,
          require: false,
          console: false,
          // less generic
          angular: false,
          WebSocket: false
        }
      },
      all: [
        'Gruntfile.js',
        'app/scripts/{,*/}*.js'
      ]
    },

    // recess
    recess: {
      options: {
        noOverqualifying: false
      },
      all: {
        src: [ 'app/styles/main.less' ]
      }
    },

    // Transform less files
    less: {
      build: {
        files: [{
          expand: true,
          cwd: 'app/styles',
          src: 'main.less',
          dest: 'build/styles',
          ext: '.css'
        }]
      }
    },

    // Add vendor prefixed styles
    autoprefixer: {
      options: {
        browsers: [ 'last 2 versions', 'iOS >= 6' ]
      },
      build: {
        files: [{
          expand: true,
          cwd: 'build/styles/',
          src: '*.css',
          dest: 'build/styles/'
        }]
      }
    },

    // Rename files for browser caching purposes
    rev: {
      dist: {
        files: {
          src: [
            'dist/scripts/{,*/}*.js',
            'dist/styles/*.css',
            'dist/images/*'
          ]
        }
      }
    },

    // Perform rewrites based on rev
    useminPrepare: {
      html: 'build/*.html',
      options: {
        dest: 'dist'
      }
    },
    usemin: {
      html: [ 'dist/*.html' ],
      css:  [ 'dist/styles/*.css' ],
      options: {
        assetsDirs: [ 'dist', 'dist/images' ]
      }
    },

    // Image minification
    imagemin: {
      options: {
        cache: false
      },
      dist: {
        files: [{
          expand: true,
          cwd: 'build/images',
          src: '*',
          dest: 'dist/images'
        }]
      }
    },

    // Prepare Angular files to be minified
    ngmin: {
      build: {
        files: [{
          expand: true,
          cwd: 'build/scripts',
          src: '{,*/}*.js',
          dest: 'build/scripts'
        }]
      }
    },

    // Build templates
    ngtemplates: {
      options: {
        module: '{{cookiecutter.project_name|lower}}'
      },
      build: {
        cwd: 'app/templates',
        src: '*.html',
        dest: 'build/scripts/templates.js'
      }
    },

    // Copy files
    copy: {
      html: {
        files: [{
          expand: true,
          cwd: 'app',
          dest: 'build',
          src: [ '*.html' ]
        }]
      },
      scripts: {
        files: [{
          expand: true,
          cwd: 'app',
          dest: 'build',
          src: [
            'scripts/{,*/}*.js'
          ]
        }]
      },
      images: {
        files: [{
          expand: true,
          cwd: 'app',
          dest: 'build',
          src: [
            'images/*'
          ]
        }]
      },
      bower: {
        files: [{
          expand: true,
          cwd: 'app',
          dest: 'build',
          src: [
            'bower_components/**/*'
          ]
        }]
      },
      dist: {
        files: [{
          expand: true,
          cwd: 'build',
          dest: 'dist',
          src: [
            '*.html'
          ]
        }]
      }
    }

  });

  grunt.registerTask('serve', [
    'build',
    'external_daemon:api',
    'external_daemon:nginx',
    'connect:livereload',
    'watch'
  ]);

  grunt.registerTask('build', function(target) {
    switch (target) {
    case 'html':
      grunt.task.run('copy:html');
      break;
    case 'templates':
      grunt.task.run('ngtemplates:build');
      break;
    case 'styles':
      grunt.task.run('recess', 'less:build', 'autoprefixer:build');
      break;
    case 'scripts':
      grunt.task.run('jshint', 'copy:scripts', 'ngmin:build');
      break;
    case 'images':
      grunt.task.run('copy:images');
      break;
    case undefined:
      grunt.task.run(
        'clean:build',
        'build:html',
        'build:templates',
        'build:styles',
        'build:scripts',
        'build:images',
        'copy:bower'
      );
      break;
    default:
      grunt.util.error('unknown target ' + target + ' for build');
    }
  });

  grunt.registerTask('dist', [
    'clean:dist',
    'build',
    'useminPrepare',
    'imagemin',
    'copy:dist',
    'concat',
    'cssmin',
    'uglify',
    'rev',
    'usemin'
  ]);

  grunt.registerTask('default', [
    'dist'
  ]);
};
