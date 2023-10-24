// const coap = require('coap');
const chalk = require('chalk');
// const req1 = coap.request({
//     hostname: '10.42.0.100',
//     port: 5683,
//     method: 'GET',
//     confirmable: false,
//     retransmitTimeout: 500,
//     pathname: 'Espressif',
//     options: {
//     //   Size2: 259,
//       Block2: true,
//     }
//   });
//   req1.on('response', function (res) {
//     console.log(`\n GET request received response:`);
//     res.pipe(process.stdout);
//     res.on('end', function () {
//       console.log('\nResponse ended');
//     //   req1.end(`\n GET request payload`);
//     });
//   });
  
//   req1.on('error', function (err) {
//     console.error(`\n GET request encountered error: ${err.message}`);
//   });
  
//   req1.end(`\n GET request payload`);
  
//   req1.on('response', function (res) {
//     console.log(`\n GET request received response:`);
//     res.pipe(process.stdout);
//     // while(!(res.pipe(process.sedout))){
//     //     continue;
//     //     console.log('havnt yet')
//     // }
//   });
  
//   req1.on('error', function (err) {
//     console.error(`\n GET non_confirmable /hello request #${count} encountered error: ${err.message}`);
//   });
//   req1.end(`\n GET request  payload`);

function send_get_request(n) {
  // const confirmable_or_not = true_false_lst[Math.floor(Math.random() * 2)];
  // const real_pathname = pathname_lst[Math.floor(Math.random() * 2)];
  // const block2_or_not = true_false_lst[Math.floor(Math.random() * 2)];
  // console.log(chalk.red("\n Sending GET " + n + " request with confirmable_or_not: ", confirmable_or_not, " to: ", real_pathname, " with block2: ", block2_or_not));

  const coap = require('coap');
  const req1 = coap.request({
      hostname: '10.47.0.1',
      port: 5683,
      method: 'get',
      confirmable: false,
      retrySend: false,
      retransmitTimeout: 500,
      pathname: '/Espressif',
      retransmit: false,
      options: {
          //   Size2: 259,
          Block2: true,
      }
  });

  req1.on('response', function (res) {
      console.log(chalk.green('\n GET ' + n + ' request received response: '));
      res.pipe(process.stdout);
      res.on('end', function () {
          console.log('\nResponse ended\n');
        //   req1.end(`\n GET request payload`);
        });
  });

  req1.on('error', function (err) {
      console.error(`\n GET Request encountered error: ${err.message}`);
  });

  req1.end("\n GET " + n + " request payload: ");
}

send_get_request(1)