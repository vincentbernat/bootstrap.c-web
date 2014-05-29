angular.module('{{cookiecutter.small_prefix}}.controllers', []);

/* TODO:3509 Implement a useful main controller. You can implement
 * TODO:3509 additional controllers in a dedicated file too.
 */
angular.module('{{cookiecutter.small_prefix}}.controllers')
  .controller('MainCtrl', function($scope) {
    'use strict';

    $scope.who = 'there';
  });
