const coap = require('coap')
const server = coap.createServer()

server.on('request', (req, res) => {
  res.end('Hello ' + req.url.split('/')[1] + '\n')
})

// the default CoAP port is 5683
server.listen(() => {
	console.log("server started at port 5683")
}) 
