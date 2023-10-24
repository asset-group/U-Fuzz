// const coap = require('coap')
const coap = require('coap');

let count = 0;

setInterval(() => {
  count++;
  // console.log("\n count: ", count%10);

  // GET request
  if (count % 10 === 1) {
    const req = coap.request({
      hostname: '10.42.0.156',
      port: 5683,
      method: 'GET',
      confirmable: true,
      pathname: '/Espressif',
      retrySend: false,
      retransmitTimeout: 300,
    });

    req.on('response', function (res) {
      console.log(`\n GET request #${count} received response:`);
      res.pipe(process.stdout);
    });

    req.on('error', function (err) {
      console.error(`\n GET confirmable /Espressif request #${count} encountered error: ${err.message}`);
    });

    req.end(`\n GET request #${count} payload`);
  } else if (count % 10 === 2) {
    const req = coap.request({
      hostname: '10.42.0.156',
      port: 5683,
      method: 'GET',
      confirmable: false,
      retrySend: false,
      retransmitTimeout: 300,
      pathname: '/Espressif',
    });

    req.on('response', function (res) {
      console.log(`\n GET request #${count} received response:`);
      res.pipe(process.stdout);
    });

    req.on('error', function (err) {
      console.error(`\n GET non_confirmable /Espressif request #${count} encountered error: ${err.message}`);
    });

    req.end(`\n GET request #${count} payload`);
  } else if (count % 10 === 5) {
    const req = coap.request({
      hostname: '10.42.0.156',
      port: 5683,
      method: 'GET',
      confirmable: true,
      retrySend: false,
      retransmitTimeout: 300,
      pathname: '/hello',
    });

    req.on('response', function (res) {
      console.log(`\n GET request #${count} received response:`);
      res.pipe(process.stdout);
    });

    req.on('error', function (err) {
      console.error(`\n GET confirmable /hello request #${count} encountered error: ${err.message}`);
    });

    req.end(`\n GET request #${count} payload`);
  } else if (count % 10 === 6) {
    const req = coap.request({
      hostname: '10.42.0.156',
      port: 5683,
      method: 'GET',
      confirmable: false,
      retrySend: false,
      retransmitTimeout: 300,
      pathname: '/hello',
    });

    req.on('response', function (res) {
      console.log(`\n GET request #${count} received response:`);
      res.pipe(process.stdout);
    });

    req.on('error', function (err) {
      console.error(`\n GET non_confirmable /hello request #${count} encountered error: ${err.message}`);
    });

    req.end(`\n GET request #${count} payload`);
  }
  else if (count % 10 === 3) {
    // PUT request
    const req = coap.request({
      hostname: '10.42.0.156',
      port: 5683,
      method: 'PUT',
      confirmable: true,
      retrySend: false,
      retransmitTimeout: 300,
      pathname: '/Espressif',
      data: 'Is it because of the data?'
    });
    req.on('response', function (res) {
      console.log(`\n PUT request #${count} received response: `);
      res.pipe(process.stdout);
    });

    req.on('error', function (err) {
      console.error(`\n PUT confirmable /Espressif request #${count} encountered error: ${err.message}`);
    });
    req.end(`\n PUT request #${count} payload`);
  } else if (count % 10 === 4) {
    // PUT request
    const req = coap.request({
      hostname: '10.42.0.156',
      port: 5683,
      method: 'PUT',
      confirmable: false,
      retrySend: false,
      retransmitTimeout: 300,
      pathname: '/hello',
      data: 'test put',
    });
    req.on('response', function (res) {
      console.log(`\n PUT request #${count} received response: `);
      res.pipe(process.stdout);
    });

    req.on('error', function (err) {
      console.error(`\n PUT non_confirmable /hello request #${count} encountered error: ${err.message}`);
    });
    req.end(`\n PUT request #${count} payload`);
  }
  else if (count % 10 === 9) {
    // DELETE request
    const req = coap.request({
      hostname: '10.42.0.156',
      port: 5683,
      method: 'POST',
      confirmable: true,
      retrySend: false,
      retransmitTimeout: 300,
      pathname: '/Espressif',
      data: 'test Post',
    });
    req.on('response', function (res) {
      console.log(`\n POST request #${count} received response: `);
      res.pipe(process.stdout);
    });

    req.on('error', function (err) {
      console.error(`\n POST confirmable /Espressif request #${count} encountered error: ${err.message}`);
    });
    req.end(`\n POST request #${count} payload`);
  } else if (count % 10 === 0) {
    // DELETE request
    const req = coap.request({
      hostname: '10.42.0.156',
      port: 5683,
      method: 'POST',
      confirmable: false,
      retrySend: false,
      retransmitTimeout: 300,
      pathname: '/hello',
    });
    req.on('response', function (res) {
      console.log(`\n POST request #${count} received response: `);
      res.pipe(process.stdout);
    });

    req.on('error', function (err) {
      console.error(`\n POST non_confirmable /hello request #${count} encountered error: ${err.message}`);
    });
    req.end(`\n POST request #${count} payload`);
  } else if (count % 10 === 7) {
    const req = coap.request({
      hostname: '10.42.0.156',
      port: 5683,
      method: 'DELETE',
      confirmable: true,
      retrySend: false,
      retransmitTimeout: 300,
      pathname: '/Espressif',
    });

    req.on('response', function (res) {
      console.log(`\n DELETE request #${count} received response:`);
      res.pipe(process.stdout);
    });

    req.on('error', function (err) {
      console.error(`\n DELETE confirmable /Espressif request #${count} encountered error: ${err.message}`);
    });

    req.end(`\n DELETE request #${count} payload`);
  } else {
    const req = coap.request({
      hostname: '10.42.0.156',
      port: 5683,
      method: 'DELETE',
      confirmable: false,
      retrySend: false,
      retransmitTimeout: 300,
      pathname: '/hello',
    });

    req.on('response', function (res) {
      console.log(`\n DELETE request #${count} received response:`);
      res.pipe(process.stdout);
    });

    req.on('error', function (err) {
      console.error(`\n DELETE non_confirmable /hello request #${count} encountered error: ${err.message}`);
    });

    req.end(`\n DELETE request #${count} payload`);
  }
}, 100);


// const req1 = coap.request('coap://10.42.0.156/Espressif')

// // This part help to create payload for get requets
// const payload = {
//   title: 'Test Tittle',
//   body: 'Test Body'
// }

// req1.write(JSON.stringify(payload))
// setTimeout(() => {
//   req1.on('response', (res) => {
//     res.pipe(process.stdout)
//     res.on('end', () => {
//       process.exit(0)
//     })
//   })
// }, 1000)

// req1.end()

// const req2 = coap.request({
//   hostname: '10.42.0.156',
//   port: 5683,
//   pathname: '/Espressif',
//   confirmable: false,
//   method: 'PUT'
// })
// setTimeout(() => {
//   req2.on('response', (res) => {
//     res.pipe(process.stdout)
//     res.on('end', () => {
//       process.exit(0)
//     })
//   })
// }, 1000)

// req2.end()

// const req3 = coap.request({
//   hostname: '10.42.0.156',
//   port: 5683,
//   pathname: '/Espressif/hello',
//   method: 'POST'
// })
// setTimeout(() => {
//   req3.on('response', (res) => {
//     res.pipe(process.stdout)
//     res.on('end', () => {
//       process.exit(0)
//     })
//   })
// }, 1000)

// req3.end()

// const req4 = coap.request({
//   hostname: '10.42.0.156',
//   port: 5683,
//   pathname: '/Espressif/hello',
//   confirmable: false,
//   method: 'DELETE'
// })
// setTimeout(() => {
//   req4.on('response', (res) => {
//     res.pipe(process.stdout)
//     res.on('end', () => {
//       process.exit(0)
//     })
//   })
// }, 1000)

// req4.end()

