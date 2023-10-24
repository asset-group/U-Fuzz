// const payload_length_generator = Math.floor(Math.random() * 2000);
// const random_size = payload_length_generator
const chalk = require('chalk');
const size1_lst = [0, 0]
// const true_false = Math.floor(Math.random() * 2);
const true_false_lst = [true, false];
// canpous/jcoap
const pathname_lst = ['/hello', '/basic','/hello'];
// aiocoap
// const pathname_lst = ['/other/block', '/other/block','/other/block'];
// Nanocoap
// const pathname_lst = ['/.well-known/core', '/test','/test'];
// CoAPthon
// const pathname_lst = ['/basic', '/child','/separate','/big'];

// const condition = true;
let count = 0;


function send_get_request(n) {
    const confirmable_or_not = true_false_lst[Math.floor(Math.random() * 2)];
    const real_pathname = pathname_lst[Math.floor(Math.random() * 4)];
    const block2_or_not = true_false_lst[Math.floor(Math.random() * 2)];
    console.log(chalk.red("\n Sending GET " + n + " request with confirmable_or_not: ", confirmable_or_not, " to: ", real_pathname, " with block2: ", block2_or_not));

    const coap = require('coap');
    const req1 = coap.request({
        hostname: '10.13.210.82',
        port: 5683,
        // iface: 'veth5',
        method: 'get',
        confirmable: confirmable_or_not,
        retrySend: false,
        retransmitTimeout: 500,
        pathname: real_pathname,
        retransmit: false,
        options: {
            //   Size2: 259,
            Block2: block2_or_not,
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

function send_fetch_request(n) {
    const confirmable_or_not = true_false_lst[Math.floor(Math.random() * 2)];
    const real_pathname = pathname_lst[Math.floor(Math.random() * 4)];
    const block2_or_not = true_false_lst[Math.floor(Math.random() * 2)];
    console.log(chalk.red("\n Sending FETCH " + n + " request with confirmable_or_not: ", confirmable_or_not, " to: ", real_pathname, " with block2: ", block2_or_not));
    const coap = require('coap');
    const req1 = coap.request({
        hostname: '10.13.210.82',
        port: 5683,
        // iface: 'veth5',
        method: 'fetch',
        confirmable: confirmable_or_not,
        retrySend: false,
        retransmitTimeout: 500,
        pathname: real_pathname,
        retransmit: false,
        options: {
            //   Size2: 259,
            Block2: block2_or_not,
        }
    });

    req1.on('response', function (res) {
        console.log("\n FETCH " + n + " request received response: ");
        res.pipe(process.stdout);
        res.on('end', function () {
            console.log('\nResponse ended\n');
          //   req1.end(`\n GET request payload`);
          });
    });

    req1.on('error', function (err) {
        console.error(`\n FETCH Request encountered error: ${err.message}`);
    });

    req1.end("\n FETCH " + n + " request payload: ");
}

function send_put_request(n) {
    const confirmable_or_not = true_false_lst[Math.floor(Math.random() * 2)];
    const real_pathname = pathname_lst[Math.floor(Math.random() * 4)];
    const block1_or_not = true_false_lst[Math.floor(Math.random() * 2)];
    var payload_length = 0;
    if (block1_or_not) {
        payload_length = Math.floor(Math.random() * 1024);
    } else {
        payload_length = 32;
    }
    // size1_lst[0] = payload_length;
    // size1_lst[1] = Math.floor(Math.random() * 2000);
    const samplePayload = 'h'.repeat(payload_length);
    // size1_factor = Math.floor(Math.random()*64)

    // const real_size1 = size1_lst[Math.floor(Math.random() * 2)]
    console.log(chalk.red("\n Sending PUT " + n + " request with confirmable_or_not: ", confirmable_or_not, " to: ", real_pathname, " with block1: ", block1_or_not, "with size1: ", 16));
    console.log('\n samplePayload.length: ', samplePayload.length);

    const coap = require('coap');
    if (block1_or_not) {

        const req = coap.request({
            host: '10.13.210.82',
            pathname: real_pathname,
            port: 5683,
            // iface: 'veth5',
            method: 'put',
            confirmable: confirmable_or_not,
            retransmitTimeout: 500,
            retrySend: false,
            retransmit: false,
            options: {
                // size1 indicates the total size of the payload
                Size1: 16,
                Block1: true,
                "Content-Format": "text/plain",
                // "Content-Format": 'number',
                // set block size to maximum size of each block
            }
        });

        req.on('response', function (res) {
            console.log(chalk.green("\n PUT " + n + "  request received response:"));
            res.on('data', function (chunk) {
                console.log('\n' + chunk.toString());
            });
        });
        req.on('error', function (err) {
            console.error(`\n PUT request encountered error: ${err.message}`);
        });

        req.write(samplePayload);
        req.end();
    } else {
        const req = coap.request({
            host: '10.13.210.82',
            pathname: real_pathname,
            port: 5683,
            // iface: 'veth5',
            method: 'put',
            confirmable: confirmable_or_not,
            retrySend: false,
            retransmit: false,
            options: {
                // size1 indicates the total size of the payload
                //   Size1: samplePayload.length,
                //   Block1: true,
                //   "Content-Format": "text/plain",
                // "Content-Format": 'number',
                // set block size to maximum size of each block
            }
        });

        req.on('response', function (res) {
            console.log(chalk.green("\n PUT " + n + " request received response: "));
            res.on('data', function (chunk) {
                console.log('\n ' + chunk.toString());
            });
        });
        req.on('error', function (err) {
            console.error(`\n PUT request encountered error: ${err.message}`);
        });

        req.write(samplePayload);
        req.end();

    }
}

function send_post_request(n) {
    const confirmable_or_not = true_false_lst[Math.floor(Math.random() * 2)];
    const real_pathname = pathname_lst[Math.floor(Math.random() * 4)];
    const block1_or_not = true_false_lst[Math.floor(Math.random() * 2)];
    var payload_length = 0;
    if (block1_or_not) {
        payload_length = Math.floor(Math.random() * 1024);
    } else {
        payload_length = 32;
    }
    // size1_factor = Math.floor(Math.random()*64)
    // size1_lst[0] = 32;
    // size1_lst[1] = Math.floor(Math.random() * 1024);
    const samplePayload = 'h'.repeat(payload_length);
    // const real_size1 = size1_lst[Math.floor(Math.random() * 2)]
    console.log(chalk.red("\n Sending POST " + n + " request with confirmable_or_not: ", confirmable_or_not, " to: ", real_pathname, " with block1: ", block1_or_not, " with size1: ", 16));
    console.log('\n samplePayload.length: ', samplePayload.length);
    const coap = require('coap');
    if (block1_or_not) {
        const req = coap.request({
            host: '10.13.210.82',
            pathname: real_pathname,
            port: 5683,
            // iface: 'veth5',
            method: 'post',
            confirmable: confirmable_or_not,
            retransmitTimeout: 500,
            retrySend: false,
            retransmit: false,
            options: {
                Size1: 16,
                Block1: true,

                // set block size to maximum size of each block
            }
        });

        req.on('response', function (res) {
            console.log(chalk.green('\n POST ' + n + ' request received response:'));
            res.on('data', function (chunk) {
                console.log('\n ' + chunk.toString());
            });
        });
        req.on('error', function (err) {
            console.error(`\n POST request encountered error: ${err.message}`);
        });

        req.write(samplePayload);
        req.end();
    } else {
        const req = coap.request({
            host: '10.13.210.82',
            pathname: real_pathname,
            port: 5683,
            // iface: 'veth5',
            method: 'post',
            confirmable: confirmable_or_not,
            retrySend: false,
            retransmit: false,
            options: {
                //   Size1: real_size1,
                //   Block1: true,

                // set block size to maximum size of each block
            }
        });

        req.on('response', function (res) {
            console.log(chalk.green('\n POST ' + n + ' request received response:'));
            res.on('data', function (chunk) {
                console.log('\n ' + chunk.toString());
            });
        });
        req.on('error', function (err) {
            console.error(`\n POST request encountered error: ${err.message}`);
        });

        req.write(samplePayload);
        req.end();
    }
}

function send_delete_request(n) {
    const confirmable_or_not = true_false_lst[Math.floor(Math.random() * 2)];
    const real_pathname = pathname_lst[Math.floor(Math.random() * 4)];
    console.log(chalk.red("\n Sending DELETE " + n + " request with confirmable_or_not: ", confirmable_or_not, " to: ", real_pathname));
    const coap = require('coap');
    const req = coap.request({
        hostname: '10.',
        port: 5683,
        // iface: 'veth5',
        // iface: 'veth5',
        method: 'DELETE',
        confirmable: confirmable_or_not,
        retrySend: false,
        retransmitTimeout: 500,
        retransmit: false,
        pathname: real_pathname,
    });

    req.on('response', function (res) {
        console.log(chalk.green('\n DELETE request ' + n + ' received response:'));
        res.pipe(process.stdout);
    });

    req.on('error', function (err) {
        console.error(`\n DELETE request encountered error: ${err.message}`);
    });

    req.end();
}

// request type: GET, PUT, POST, DELETE, large PUT, large GET, large POST, FETCH, large FETCH
// support option right now, Size1, Size2, Block1, Block2, payloadlength

// main loop to send request

setInterval(() => {
    count++
    console.log("\n Sending request " + count);
    setTimeout(() => {
        const randomNUm = Math.floor(Math.random() * 3);
        switch (randomNUm) {
            case 0:
                try {
                    send_get_request(count);
                } catch (error) {
                    console.error(`sending get request number #${count} encountered error : ${error.message}`);
                }
                break;
            case 1:
                try {
                    send_put_request(count);
                } catch (error) {
                    console.error(`sending get request number #${count} encountered error : ${error.message}`);
                }
                break;
            case 2:
                try {
                    send_post_request(count);
                } catch (error) {
                    console.error(`sending post request number #${count} encountered error : ${error.message}`);
                }
                break;
            // case 3:
            //     try {
            //         send_delete_request(count);
            //     } catch (error) {
            //         console.error(`sending delete request number #${count} encountered error : ${error.message}`);
            //     }
            //     // send_delete_request(count);
            //     break;
            // case 4:
            //     try {
            //         send_fetch_request(count);
            //     } catch {
            //         console.error(`sending fetch request number #${count} encountered error : ${error.message}`);
            //     }
                // send_fetch_request(count);
                // break;
            default:
                break;
        }
    }, 1000)
}, 1000)
