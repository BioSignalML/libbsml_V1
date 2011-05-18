import threading

import json
import hashlib

BUFFER_SIZE = 4096


ERROR_UNEXPECTED_TRAILER = 1
ERROR_MISSING_HEADER_LF  = 2
ERROR_MISSING_TRAILER    = 3
ERROR_INVALID_CHECKSUM   = 4
ERROR_MISSING_TRAILER_LF = 5

ERROR_TEXT = { ERROR_UNEXPECTED_TRAILER: 'Unexpected block trailer',
               ERROR_MISSING_HEADER_LF: 'Missing LF on header',
               ERROR_MISSING_TRAILER: 'Missing block trailer',
               ERROR_INVALID_CHECKSUM: 'Invalid block checksum',
               ERROR_MISSING_TRAILER_LF: 'Missing LF on trailer',
             }


CHECKSUM_STRICT = 1
CHECKSUM_CHECK  = 2
CHECKSUM_IGNORE = 3
CHECKSUM_NONE   = 4


class Block(object):
#===================

  def __init__(self, blockno, type, format, header, content):
  #----------------------------------------------------------
    self.blockno = blockno
    self.type = type
    self.format = format
    self.header = header
    self.content = content

  def __str__(self):
  #-----------------
    return ("BLOCK %d: Type=%c, Format='%s', Header=%s, Length=%d"
           % (self.blockno, self.type, self.format, self.header, len(self.content)))


class BlockReader(object):
#=========================

  def __init__(self, fh, error_handler, checksum=CHECKSUM_CHECK):
  #--------------------------------------------------------------
    self._file = fh
    self._error = error_handler
    self._checksum = checksum

  def __iter__(self):
  #------------------
    state = 0
    blockno = -1
    pos = datalen = 0
    while True:
      if datalen <= 0:
        data = self._file.read(BUFFER_SIZE)
        pos = 0
        datalen = len(data)
        if datalen <= 0: break

      if   state == 0:                 # Looking for a block
        next = data[pos:].find('#')
        if next >= 0:
          pos += (next + 1)
          datalen -= 1
          checksum = hashlib.md5()
          checksum.update('#')
          state = 1
        else:
          datalen = 0
            
      elif state == 1:                 # Getting block type 
        type = data[pos]
        pos += 1
        datalen -= 1
        if type != '#':
          checksum.update(type)
          blockno += 1
          length = 0
          state = 2
        else:
          self._error(blockno, ERROR_UNEXPECTED_TRAILER)
          state = 0

      elif state == 2:                 # Getting header length
        while datalen > 0 and data[pos].isdigit():
          length = 10*length + int(data[pos])
          checksum.update(data[pos])
          pos += 1
          datalen -= 1
        if datalen > 0:
          jsonhdr = [ ]
          state = 3

      elif state == 3:                 # Getting header JSON
        while datalen > 0 and length > 0:
          jsonhdr.append(data[pos:pos+length])
          checksum.update(data[pos:pos+length])
          delta = min(length, datalen)
          pos += delta
          datalen -= delta
          length -= delta
        if length == 0:
          header = json.loads(''.join(jsonhdr)) if len(jsonhdr) else { }
          state = 4

      elif state == 4:                 # Checking header LF
        if data[pos] == '\n':
          pos += 1
          datalen -= 1
          checksum.update('\n')
          length = header.pop('length', 0)
          chunks = [ ]
          state = 5
        else:
          self._error(blockno, ERROR_MISSING_HEADER_LF)
          state = 0

      elif state == 5:                 # Getting content
        while datalen > 0 and length > 0:
          chunks.append(data[pos:pos+length])
          checksum.update(data[pos:pos+length])
          delta = min(length, datalen)
          pos += delta
          datalen -= delta
          length -= delta
        if length == 0:
          content = ''.join(chunks)
          length = 2
          state = 6

      elif state == 6:                 # Getting trailer
        if data[pos] == '#':
          pos += 1
          datalen -= 1
          length -= 1
          if length == 0: state = 7
        else:
          self._error(blockno, ERROR_MISSING_TRAILER)
          state = 0

      elif state == 7:                 # Checking for checksum
        if data[pos] != '\n' and self._checksum != CHECKSUM_NONE:
          length = 32
          state = 8
        else:
          state = 9
        checks = [ ]

      elif state == 8:                 # Getting checksum
        while datalen > 0 and length > 0:
          checks.append(data[pos:pos+length])
          delta = min(length, datalen)
          pos += delta
          datalen -= delta
          length -= delta
        if length == 0: state = 9

      elif state == 9:                 # Checking for final LF
        if ((self._checksum == CHECKSUM_STRICT
          or self._checksum == CHECKSUM_CHECK and len(checks))
         and ''.join(checks) != checksum.hexdigest()):
          self._error(blockno, ERROR_INVALID_CHECKSUM)
        elif data[pos] == '\n':
          pos += 1
          datalen -= 1
          format = header.pop('format', '')
          yield Block(blockno, type, format, header, content)
        else:
          self._error(blockno, ERROR_MISSING_TRAILER_LF)
        state = 0



class BlockWriter(object):
#=========================

  def __init__(self, fh):
  #----------------------
    self._file = fh


  def write(self, type, format='', header=None, content='', check=False):
  #======================================================================

    if header is None: header = { }
    elif not isinstance(header, dict):
      raise Exception('Block header must be a dictionary')
    elif type == '#':
      raise Exception('Block type of "#" is reserved')

    length = len(content)
    header['length'] = length
    if format: header['format'] = format
    jsonhdr = json.dumps(header, separators=(',',':'))

    checksum = hashlib.md5() if check else None

    hdr = ('#%c' % str(type)[0] if type else ' ') + str(len(jsonhdr)) + jsonhdr + '\n'
    self._file.write(hdr)
    if checksum: checksum.update(hdr)

    n = 0
    while n < length:
      self._file.write(content[n:n+BUFFER_SIZE])
      if checksum: checksum.update(content[n:n+BUFFER_SIZE])
      n += BUFFER_SIZE

    self._file.write('##')
    if checksum: self._file.write(checksum.hexdigest())
    self._file.write('\n')



class StreamReader(threading.Thread):
#====================================

  def __init__(self, fh, processor, error_handler, **kwds):
  #--------------------------------------------------------
    threading.Thread.__init__(self, **kwds)
    self._reader = BlockReader(fh, error_handler)
    self._process = processor


  def start(self):
  #---------------
    for block in self._reader:
      self._process(block)



class SimpleStreamReader(object):
#================================


  @staticmethod
  def process(block):
  #------------------
    pass

  @staticmethod
  def error(blockno, errno):
  #-------------------------
    pass


  def __init__(self, fh, metadata, data_process):
  #----------------------------------------------
    self._stream = StreamReader(fh, self.process, self.error)


"""
  type     STREAM_METADATA, etc
  format   'turtle', etc
  encoding 'UTF-8', etc
  

  @property
  def length(self):
    return self._data.get('length', 0)
  

  def __getitem__(self, key):
    return self._data.get(key)

  def get(self, key, default=None):
    return self._data.get(key, default)



  header['attribute'] 
"""

def processor(block):
#====================
  pass

def error_handler(blockno, errno):
#=================================
  pass


if __name__ == '__main__':
#=========================

  import sys

  if len(sys.argv) > 1:
    if sys.argv[1][0] == 'o':
      bw = BlockWriter(sys.stdout)
#  bw.write('D', content='some content', check=True) 
      bw.write('M', format='turtle', content='<a> <b> 1 .')

      while True:
        d = sys.stdin.read(8192)
        if d == '': break
        bw.write('D', content=d, check=True) 

    elif sys.argv[1][0] == 'i':

      def printblock(blk):
        print blk

      def printerror(blkno, errno):
        print 'ERROR %d in BLOCK %d (%s)' % (blkno, errno, ERROR_TEXT[errno])

      sr = StreamReader(sys.stdin, printblock, printerror)
      sr.start()

      while sr.is_alive():
        sleep(100)
