function MQTTconnect() {
    clientID = "clientID-" + parseInt(Math.random() * 100);
    host  = document.getElementById("host").value;
    port  = document.getElementById("port").value;
    
    const options = {
      // Clean session
      clean: true,
      connectTimeout: 4000,
      // Auth
      clientId: clientID,
      username: 'emqx_test',
      password: 'emqx_test',
    }

    // Print output for the user in the messages div
    console.log('Connecting to: ' + host + ' on port: ' + port + '</span><br/> - '+`mqtt://${host}:${port}`)
    document.getElementById("messages").innerHTML += '<span>Connecting to: ' + host + ' on port: ' + port + '</span><br/>';
    document.getElementById("messages").innerHTML += '<span>Using the following client value: ' + clientID + '</span><br/>';

    // Initialize client connection
    const client = client.connect(`mqtt://${host}:${port}`, options)
    
    client.on('connect', () => {
      console.log('Connected')
      client.subscribe('test', function (err) {
        if (!err) {
          client.publish('test', 'Hello mqtt')
        }
      })
    })
    // client.on('message', function (topic, message) {
    //   // message is Buffer
    //   console.log(message.toString())
    //   client.end()
    // })
    // client.on('connect', function (connack) {console.log('Connected')})
    client.on('reconnect',  () => console.log('Reconnecting...'))
    client.on('close',      () => console.log('Disconnected'))
    client.on('disconnect', packet => console.log(packet))
    client.on('offline',    () => console.log('offline'))
    client.on('error',      error => console.log(error))
    client.on('message',    function (topic, payload, packet) {
        // Payload is Buffer
        console.log(`Topic: ${topic}, Message: ${payload.toString()}, QoS: ${packet.qos}`)
    })
    
}

function Publish(topic, message) {
    // Send a test message with QoS of 0 to the testtopic
    client.publish(topic, message, { qos: 0, retain: false }, function (error) {
        if (error) {
        console.log(error)
        } else {
        console.log('Published')
        }
    })
}

function Subscribe(topic) {
    // Subscribe to a topic named testtopic with QoS 0
    client.subscribe(topic, { qos: 0 }, function (error, granted) {
        if (error) {
          console.log(error)
        } else {
          console.log(`${granted[0].topic} was subscribed to topic ${topic}`)
        }
    })
}

function Unsubscribe(topic) {
    // Unsubscribe to a topic named testtopic
    client.unsubscribe(topic, function (error) {
        if (error) {
        console.log(error)
        } else {
        console.log('Unsubscribed from '+topic)
        }
    })
}