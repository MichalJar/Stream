import os
import sys
import subprocess
import struct
import array
import time

import socket
import os

class Client:
    def __init__(self, pathAddress):
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.sock.connect(pathAddress)

    def sendBytes(self, bs):
        self.sock.send(bs)

    def getBytes(self, numbs):
        bs = b''
        while numbs > len(bs):
            bs += self.sock.recv(numbs - len(bs))
        
        return bs
    def close(self):
        self.sock.close()

class Link:
    def __init__(self, a, b, lenght):
        self.a = a
        self.b = b
        self.lenght = lenght

def getSingleByte(intVal):
    b =  bytes([intVal])
    if len(b) > 1:
        raise TypeError
    return b

def intsToBytes(ints):
    buffer = array.array('i',ints)
    return buffer.tobytes()

def bytesToInts(bs):
    if type(bs) != bytes:
        raise TypeError
    l = []
    for i in range(int(len(bs) / 4)):
        l.append(struct.unpack_from('i', bs, i*4)[0])
    return l

def bytesToLinks(bs):
    if type(bs) != bytes:
        raise TypeError
    links = []
    num = int(len(bs) / (4 + 4 + 8))
    for i in range(num):
        x = struct.unpack_from('iid', bs, i*(4+4+8) )
        links.append(Link(x[0], x[1], x[2]))
    return links

def safeUnixCall(call, path):
    try:
        return call(path)
    except FileExistsError as e:
        print(e)
    except:
        pass

def safeUnixCallRun(call, path, flag):
    try:
        return call(path, flag)
    except FileExistsError:
        print(e)
    except:
        pass

class SubgraphEngine:

    def __init__(self, execPath):
        self.execPath = execPath
        self.socketPath = "/tmp/subgraphengine/socket"
        self.SEND_ELEMS       = 3
        self.GET_MODEL        = 4
        self.REMOVE_OLD_CHUNK = 5
        self.CLOSE            = 6
        self.PRINT            = 7

        try:
            os.mkdir("/tmp/subgraphengine")
        except:
            print("probably directory /tmp/subgraphengine is already created")

        self.process = subprocess.Popen([self.execPath, self.socketPath])
        time.sleep(1)
        self.connector = Client(self.socketPath)

    def sendOrder(self, order):
        o = getSingleByte(order)
        self.connector.sendBytes(o)

    def getDuration(self):
        d = self.connector.getBytes(4)        
        return bytesToInts(d)[0]

    def addElems(self,x,y):
        time.sleep(0.1)
        
        self.sendOrder(self.SEND_ELEMS)
        l = []
        for i in range(0,len(x)):
            l.append(x[i])
            l.append(y[i])
        
        self.connector.sendBytes(intsToBytes([len(l)]))
        self.connector.sendBytes(intsToBytes(l))
        
        time.sleep(1)
        duration = self.getDuration()

        return duration

    def getModel(self):
        time.sleep(0.1)

        self.sendOrder(self.GET_MODEL)
        # get time
        time.sleep(0.1)
        duration = self.getDuration()
        # get num of links 
        numOfLinks = bytesToInts(self.connector.getBytes(4))[0]
        # get data
        bs = self.connector.getBytes(numOfLinks*(4+4+8))
        return ( bytesToLinks(bs), duration)

    def close(self):
        self.connector.close()
        
    #def sendStupidMessage(self):
        #os.write(self.inputFifo, int.)

execPath = sys.argv[1]

sge = SubgraphEngine(execPath)

x = [0,12,3,44,53,3]
y = [2,3,2,1,6,5]

dur = sge.addElems(x,y)

(links,dur) = sge.getModel()

sge.close()