const coap = require('coap');

let count = 0;

setInterval(() => {
  count++;

  // GET request
  if (count % 2 == 1) {
    // const options = {
    //   retransmitTimeout: 300,
    // };
    const req = coap.request({
      hostname: '10.42.0.156',
      port: 5683,
      method: 'GET',
      confirmable: true,
      pathname: '/Espressif',
      // options,
      retrySend: false,
      retransmitTimeout: 300,
    });

    req.on('response', function (res) {
      console.log(`\n GET request #${count} received response:`);
      res.pipe(process.stdout);
    });

    req.on('error', function (err) {
      console.error(`\n GET /Espressif request #${count} encountered error: ${err.message}`);
    });


    req.end(`\n GET request #${count} payload`);
    // setTimeout(() => {}, 100);
  }else{
    // const options = {
    //   retransmitTimeout: 300,
    // };
    const req = coap.request({
      hostname: '10.42.0.156',
      port: 5683,
      method: 'GET',
      confirmable: true,
      pathname: '/hello',
      // options,
      retrySend: false,
      retransmitTimeout: 300,
      
    });
    // var payload = {
    //   username: 'test',
    // };
    // req.write(JSON.stringify(payload));
    req.on('response', function (res) {
      console.log(`\n GET request #${count} received response: `);
      res.pipe(process.stdout);
    });
    req.on('error', function (err) {
      console.error(`\n GET /hello request #${count} encountered error: ${err.message}`);
    });
    req.end(`\n GET request #${count} payload`);
    // setTimeout(() => {}, 100);
  }
}, 500);
