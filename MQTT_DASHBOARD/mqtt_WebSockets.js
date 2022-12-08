function mqttConnect() {
    // Generate a random client ID
    clientID = "clientID_" + parseInt(Math.random() * 100);

    // Fetch the hostname/IP address and port number from the form
    host = document.getElementById("host").value;
    port = document.getElementById("port").value;

    // Print output for the user in the messages div
    document.getElementById("messages").innerHTML += '<span>Connecting to: ' + host + ':' + port + '</span><br/>';
    document.getElementById("messages").innerHTML += '<span>Using the following client value: ' + clientID + '</span><br/>';
    console.log("Connecting to "+host+":"+port);

    client = new Paho.MQTT.Client(host, Number(port), clientID);
    let options = {
        timeout: 3,
        onSuccess: onConnect,
    };
    
    // Set callback handlers
    client.onConnectionLost = onConnectionLost;
    client.onMessageArrived = onMessageArrived;
    
    client.connect(options);
}

function mqttDisconnect(){
    client.disconnect();
    console.log("Disconnected");
    document.getElementById("messages").innerHTML += '<span>Disconnected</span><br/>';
    updateScroll(); // Scroll to bottom of window
}

function onConnect() {
    console.log("Successfully connected");
    document.getElementById("messages").innerHTML += '<span>Successfully connected!</span><br/>';
    subscribeTo("Room01");
    subscribeTo("Room02");
    subscribeTo("Room03");
    subscribeTo("esp8266/1");
    subscribeTo("AutomataHomeStates");
    subscribeTo("topico_breno_sub");
    subscribeTo("topico_breno_pub");
    subscribeTo("Temp_Q3_esp8266");
    subscribeTo("Umid_Q3_esp8266");
}

const publish = (topic, msg) => {  //takes topic and message string
    let message = new Paho.MQTT.Message(new Uint8Array(msg));
    message.destinationName = topic;
    client.send(message);
}

const publishString = (topic, msg) => {  //takes topic and message string
    let message = new Paho.MQTT.Message(msg);
    message.destinationName = topic;
    client.send(message);
}

// Subscribe to the requested topic
function subscribeTo(topic) {
    client.subscribe(topic);

    console.log("Subscribed to topic: "+topic);
    document.getElementById("messages").innerHTML += '<span>Subscribed to topic: ' + topic + '</span><br/>';
}

// Unubscribe to the requested topic
function unsubscribeFrom(topic) {
    client.unsubscribe(topic);

    console.log("Unsubscribed from topic: "+topic);
    document.getElementById("messages").innerHTML += '<span>Unsubscribed from topic: ' + topic + '</span><br/>';
}

function subscribe() {
    // Fetch the MQTT topic from the form
    topic = document.getElementById("topic").value;
    subscribeTo(topic);
}

function unsubscribe() {
    // Fetch the MQTT topic from the form
    topic = document.getElementById("topic").value;
    unsubscribeFrom(topic);
}

// Called when the client loses its connection
function onConnectionLost(responseObject) {
    console.log("Connection Lost");
    if (responseObject.errorCode !== 0) {
        console.log("onConnectionLost: " + responseObject.errorMessage);
    }
}

function hasClass(el, className) {
    if (el.classList)
        return el.classList.contains(className);
    return !!el.className.match(new RegExp('(\\s|^)' + className + '(\\s|$)'));
}
function addClass(el, className) {
    if (el.classList)
        el.classList.add(className)
    else if (!hasClass(el, className))
        el.className += " " + className;
}
function removeClass(el, className) {
    if (el.classList)
        el.classList.remove(className)
    else if (hasClass(el, className))    {
        var reg = new RegExp('(\\s|^)' + className + '(\\s|$)');
        el.className = el.className.replace(reg, ' ');
    }
}

function turnOn(el) {
    addClass(el, "green")
    removeClass(el, "red")
}
function turnOff(el) {
    addClass(el, "red")
    removeClass(el, "green")
}

function updateDisplay(States) {
    json = JSON.parse(States)
    if (json.Room == "01") {
        LAMP10 = document.getElementById("LAMP10")
        ANGLE = document.getElementById("ANGLE")
        if (json.Lamps[0]) turnOn(LAMP10)
        else turnOff(LAMP10)
        ANGLE.innerHTML = json.Servo[0] + "°";
    }
    if (json.Room == "02") {
        LAMP20 = document.getElementById("LAMP20")
        LAMP21 = document.getElementById("LAMP21")
        LAMP22 = document.getElementById("LAMP22")
        LAMP23 = document.getElementById("LAMP23")
        LOCK2 = document.getElementById("LOCK2")
        DOOR2 = document.getElementById("DOOR2")
        if (json.Lamps[0]) turnOn(LAMP20)
        else turnOff(LAMP20)
        if (json.Lamps[1]) turnOn(LAMP21)
        else turnOff(LAMP21)
        if (json.Lamps[2]) turnOn(LAMP22)
        else turnOff(LAMP22)
        if (json.Lamps[3]) turnOn(LAMP23)
        else turnOff(LAMP23)
        if (json.Locks[0]) turnOn(LOCK2)
        else turnOff(LOCK2)
        if (json.Doors[0]) turnOn(DOOR2)
        else turnOff(DOOR2)
    }
    if (json.Room == "03") {
        PARTY = document.getElementById("PARTY")
        if (json.Lamps[0]) turnOn(PARTY)
        else turnOff(PARTY)
        LAMP31 = document.getElementById("LAMP31")
        if (json.Lamps[1]) turnOn(LAMP31)
        else turnOff(LAMP31)
        LAMP32 = document.getElementById("LAMP32")
        if (json.Lamps[2]) turnOn(LAMP32)
        else turnOff(LAMP32)
        LAMP33 = document.getElementById("LAMP33")
        if (json.Lamps[3]) turnOn(LAMP33)
        else turnOff(LAMP33)
    }
}

// Called when a message arrives
function onMessageArrived(message) {
    
    if (message.destinationName == "AutomataHomeStates") {
        let json = JSON.stringify(message.payloadString);
        updateDisplay(JSON.parse(json))
        document.getElementById("messages").innerHTML += '<span>Topic: ' + message.destinationName + '  | Message: ' + message.payloadString + '</span><br/>';
    }
    else if (message.destinationName == "topico_breno_sub") {
        console.log("Message arrived: " + message.payloadString);
        document.getElementById("messages").innerHTML += '<span>Topic: ' + message.destinationName + '  | Message: ' + message.payloadString + '</span><br/>';
    }
    else if (message.destinationName == "topico_breno_pub") {
        console.log("Message arrived: " + message.payloadString);
        document.getElementById("messages").innerHTML += '<span>Topic: ' + message.destinationName + '  | Message: ' + message.payloadString + '</span><br/>';
    }  
    else if (message.destinationName == "Temp_Q3_esp8266") {
        console.log("Message arrived: " + message.payloadString);
        document.getElementById("messages").innerHTML += '<span>Topic: ' + message.destinationName + '  | Message: ' + message.payloadString + '</span><br/>';
        document.getElementById("temp").innerHTML = message.payloadString.split('C')[0] + " °C";
    } 
    else if (message.destinationName == "Umid_Q3_esp8266") {
        console.log("Message arrived: " + message.payloadString);
        document.getElementById("messages").innerHTML += '<span>Topic: ' + message.destinationName + '  | Message: ' + message.payloadString + '</span><br/>';
        document.getElementById("humi").innerHTML = message.payloadString + " %";
    } 
    else{
        console.log("Message arrived: " + message.payloadBytes);
        document.getElementById("messages").innerHTML += '<span>Topic: ' + message.destinationName + '  | Message: ' + message.payloadBytes + '</span><br/>';
    }
    updateScroll(); // Scroll to bottom of window    
}

// Updates #messages div to auto-scroll
function updateScroll() {
    var element = document.getElementById("messages");
    element.scrollTop = element.scrollHeight;
}