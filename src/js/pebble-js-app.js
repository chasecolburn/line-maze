var initialized = false;
var log = false; // set to true to get some debugging logging

function log(text) {
  if(log) {
    console.log(text)
  }
}

Pebble.addEventListener("ready", function() {
  log("ready called!");
  initialized = true;
  Pebble.sendAppMessage( { "js-initialized": 1 },
                        function(e) {
                          log("Successfully told pebble that I am ready");
                        },
                        function(e) {
                          log("Failed telling pebble that I am ready: "+ e.error.message);
                        }
                       );
});

Pebble.addEventListener("showConfiguration", function() {
  log("showing configuration");
  var shakeToCheat = localStorage.getItem("shake-to-cheat") || 1;
  Pebble.openURL('http://rawgithub.com/kesselborn/line-maze/SDK-2.0-with-shake-to-cheat-config/configuration.html?stc=' + shakeToCheat);
});

function setShakeToCheat(shakeToCheat) {
Pebble.sendAppMessage( { "shake-to-cheat": shakeToCheat},
                      function(e) {
                        log("Successfully delivered message with transactionId=" +
                            e.data.transactionId + " ... shake-to-cheat=" + options["shake-to-cheat"]);
                      },
                      function(e) {
                        log("Unable to deliver message with transactionId=" +
                            e.data.transactionId +
                              " Error is: " + e.error.message);
                      }
                     );

}

Pebble.addEventListener("appmessage", function(e) {
  var command = e.payload["command"];
  var shakeToCheat = localStorage.getItem("shake-to-cheat") || 1;

  if(command === "send-config") {
    log("sending config to pebble as requested");
    setShakeToCheat(shakeToCheat);
  }
});


Pebble.addEventListener("webviewclosed", function(e) {
  log("configuration closed");
  // webview closed
  var options = JSON.parse(decodeURIComponent(e.response));

  log("sending config to pebble");
  localStorage.setItem("shake-to-cheat", options["shake-to-cheat"]);
  setShakeToCheat(options["shake-to-cheat"]);

  log("Options = " + JSON.stringify(options));
});
