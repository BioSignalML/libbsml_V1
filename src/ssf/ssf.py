import threading

import json
import hashlib

BUFFER_SIZE = 4096



def block_error(msg):
#====================
  print 'ERROR:', msg


class StreamReader(threading.Thread):
#====================================

  def __init__(self, fh, processor, **kwds):
  #-----------------------------------------
    threading.Thread.__init__(self, **kwds)

    self._reader = BlockReader(fh)
    self._process = processor


class BlockReader(object):
#=========================

  def __init__(self, fh):
  #----------------------
    self._file = fh

  def __iter__(self):
  #------------------
    state = 0
    blockno = 0
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
          length = 0
          state = 2
        else:
          block_error('Unexpected block trailer')
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
          header = json.loads(''.join(jsonhdr))
          state = 4

      elif state == 4:                 # Getting header LF
        while datalen > 0:
          char = data[pos]
          pos += 1
          datalen -= 1
          if char == '\n':
            checksum.update(char)
            length = header.get('length', 0)
            chunks = [ ]
            state = 5
            break

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
          block_error('Missing block trailer')
          state = 0

      elif state == 7:                 # Checking for checksum
        if data[pos] != '\n':
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
        if data[pos] == '\n':
          pos += 1
          datalen -= 1
          if len(checks) and ''.join(checks) != checksum.hexdigest():
            block_error('Invalid block checksum')
          yield (blockno, type, header.get('format', ''), header, content)
          blockno += 1
        else:
          block_error('Missing LF on trailer')
        state = 0



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


if __name__ == '__main__':
#=========================

  import sys

#  bw.write('D', content='some content', check=True) 
#  bw.write('M', format='turtle', content='<a> <b> 1 .') 
  if len(sys.argv) > 1:
    if sys.argv[1][0] == 'o':
      bw = BlockWriter(sys.stdout)
      while True:
        d = sys.stdin.read(8192)
        if d == '': break
        bw.write('D', content=d, check=True) 

    elif sys.argv[1][0] == 'i':
      br = BlockReader(sys.stdin)
      for b in br:
        print b

