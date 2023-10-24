import logging
import asyncio
import random

from aiocoap import *

logging.basicConfig(level=logging.INFO)

def random_number_generator(start,end):
    return random.randint(start,end)

async def put():
    """Perform a single PUT request to localhost on the default port, URI
    "/other/block". The request is sent 2 seconds after initialization.
    The payload is bigger than 1kB, and thus sent as several blocks."""
    context = await Context.create_client_context()
    # This await will hold the coroutine util it receives all the response 
    await asyncio.sleep(1)
    msgType = random_number_generator(0,1)
    payload_legth = random_number_generator(0,30)
    payload = b"The quick brown fox jumps over the lazy dog.\n" * payload_legth
    request = Message(code=PUT, payload=payload, uri="coap://10.13.210.82/other/block",mtype=msgType)
    # s1 = random_number_generator(1,4)
    # request.opt.size1= s1
    # print("--------------------------------------This is size 1 "+str(s1)+"----------------------------\n")
    # request.opt.block1 = 
    try:
        response = await asyncio.wait_for(context.request(request).response,timeout=0.5)
    except RuntimeError as e1:
        print("Server Hang for put\n")
        print(e1)
    except Exception as e:
        print(e)
    else:
        print('Result: %s\n%r'%(response.code, response.payload))

async def get():
    protocol = await Context.create_client_context()
    # kwargs = {'remote':'10.13.210.82'}
    request = Message(code=GET,uri='coap://10.13.210.82/other/block')
    # request
    # request.opt.uri_host = '10.13.210.82'
    # request.opt.uri_path = '/whoami'

    try:
        response = await asyncio.wait_for(protocol.request(request).response,timeout=0.5)
    except RuntimeError as e1:
        print("Server Hang for get \n")
        print(e1)
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
    else:
        print('Result: %s\n%r'%(response.code, response.payload))

if __name__ == "__main__":
    # asyncio.run(get())
    while(True):
        x = random_number_generator(0,4)
        if(x>=2):
            asyncio.run(put())
            # await asyncio.wait_for(put(),timeout=)
        else:
            asyncio.run(get())