/** @jsx React.DOM */
var React = require('react')
  , Router = require('react-router')
  , Navigation = Router.Navigation
  , Route = Router.Route
  , NotFoundRoute = Router.NotFoundRoute
  , DefaultRoute = Router.DefaultRoute
  , Link = Router.Link
  , Redirect = Router.Redirect
  , RouteHandler = Router.RouteHandler
  ;

var App = React.createClass({
  mixins: [ Navigation ],
  render: function() {
    return (
      <div>
          <h1>Hello, world!</h1>
          <RouteHandler />
      </div>
    );
  }
});

var Welcome = require('./Welcome.jsx')
  ;

var routes = (
  <Route name="main" path="/" handler={App}>
      <Route name="welcome"
             handler={Welcome} />
      <Redirect to="welcome"/>
  </Route>
);

Router.run(routes, function(Handler) {
  React.render(<Handler/>, document.getElementById('main'));
});
