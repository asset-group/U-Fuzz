const coap = require('coap');
const samplePayload = 'h'.repeat(64) + 'e'.repeat(64) + 'l'.repeat(64)  + 'o'.repeat(60)+'haha';
// const samplePayload = 'h'.repeat(10);


// samplePayload = samplePayload + 'end'
console.log('samplePayload.length: ', samplePayload.length);

const req = coap.request({
  host: '10.42.0.232',
  pathname: '/hello',
  port: 5683,
  method: 'POST',
  confirmable: true,
  options: {
    Size1: samplePayload.length,
    Block1: true,

     // set block size to maximum size of each block
  }
});

req.on('response', function (res) {
  console.log('POST request received response:');
  res.on('data', function (chunk) {
    console.log(chunk.toString());
  });
});

req.write(samplePayload);
req.end();




