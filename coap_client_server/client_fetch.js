const coap = require('coap');

const req1 = coap.request({
    hostname: '10.42.0.156',
    port: 5683,
    method: 'fetch',
    confirmable: true,
    retransmitTimeout: 300,
    pathname: '/hello',
    options: {
    //   Size2: 259,
    //   Block2: true,
    }
  });
  
  req1.on('response', function (res) {
    console.log(`\n FETCH request received response:`);
    res.pipe(process.stdout);
  });
  
  req1.on('error', function (err) {
    console.error(`\n FETCH non_confirmable /hello request #${count} encountered error: ${err.message}`);
  });
  
  req1.end(`\n FETCH request  payload`);