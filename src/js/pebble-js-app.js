var initialized = false;

Pebble.addEventListener("ready", function() {
  console.log("ready called!");
  initialized = true;
});

Pebble.addEventListener("showConfiguration", function() {
  console.log("showing configuration");
  var shakeToCheat = 1; // get current setting from pebble
  Pebble.openURL('http://rawgithub.com/kesselborn/line-maze/SDK-2.0-with-shake-to-cheat-config/configuration.html?stc=' + shakeToCheat);
});

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("configuration closed");
  // webview closed
  var options = JSON.parse(decodeURIComponent(e.response));
  console.log("Options = " + JSON.stringify(options));
});
