const coap = require('coap');
const samplePayload = 'h'.repeat(64) + 'e'.repeat(64) + 'l'.repeat(64)  + 'o'.repeat(60)+'haha';
// const samplePayload = 'h'.repeat(10);


// samplePayload = samplePayload + 'end'
console.log('samplePayload.length: ', samplePayload.length);

const req = coap.request({
  host: '10.42.0.156',
  pathname: '/Espressif',
  port: 5683,
  method: 'put',
  confirmable: true,
  options: {
    // size1 indicates the total size of the payload
    Size1: samplePayload.length,
    Block1: true,
    "Content-Format": "text/plain",
    // "Content-Format": 'number',
     // set block size to maximum size of each block
  }
});

req.on('response', function (res) {
  console.log('PUT request received response:');
  res.on('data', function (chunk) {
    console.log(chunk.toString());
  });
});

req.write(samplePayload);
req.end();




