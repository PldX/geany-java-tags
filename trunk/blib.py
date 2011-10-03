import hashlib
import pickle
import os
import os.path
import subprocess

class Object(object):
  pass

def RunCmd(args):
  print 'Running: %s' % ' '.join(args)
  out = subprocess.Popen(args, stdout=subprocess.PIPE).communicate()[0].replace('\n', ' ')
  print 'Output: %s\n\n' % out
  return out

def BaseName(filename):
  if isinstance(filename, Buildable):
    filename = filename.output
  return os.path.basename(filename)

def FileName(filename):
  return os.path.splitext(BaseName(filename))[0]

def BuildPath(path):
  return os.path.join('build', path)

def LoadConfig():
  config_path = BuildPath('.config')
  if not os.path.exists(config_path):
    return None
  return pickle.load(open(config_path, 'r'))

def SaveConfig(config):
  config_path = BuildPath('.config')
  if not os.path.exists('build'):
    os.makedirs('build')
  pickle.dump(config, open(config_path, 'w'))

def LoadState():
  state_path = BuildPath('.state')
  if not os.path.exists(state_path):
    return Object()
  return pickle.load(open(state_path, 'r'))

def SaveState(state):
  state_path = BuildPath('.state')
  if not os.path.exists('build'):
    os.makedirs('build')
  pickle.dump(state, open(state_path, 'w'))

def RequiresBuild(*files):
  state = LoadState()
  if not hasattr(state, 'md5s'):
    return True
  md5s = state.md5s.get(files[0], [])
  if len(files) != len(md5s):
    return True
  for (f, md5) in zip(files, md5s):
    if ComputeMd5(f) != md5:
      return True
  return False

def ReportBuilt(*files):
  state = LoadState()
  if not hasattr(state, 'md5s'):
    state.md5s = {}
  state.md5s[files[0]] = [ComputeMd5(f) for f in files]
  SaveState(state)

def ComputeMd5(filename):
  if not os.path.exists(filename):
    return ""
  f = open(filename)
  m = hashlib.md5()
  m.update(f.read())
  return m.hexdigest()

class PkgConfig(object):
  def __init__(self, module):
    self.module = module
  def CheckMinVersion(self, min_version):
    mod_version = RunCmd(['pkg-config', '--modversion', self.module])
    if mod_version < min_version:
      raise "Module %s to old: %s. Mininum required: %s" % (self.module, mod_version, min_version)
  def CFlags(self):
    return filter(NonEmpty, RunCmd(['pkg-config', '--cflags', self.module]).split(' '))
  def Libs(self):
    return filter(NonEmpty, RunCmd(['pkg-config', '--libs', self.module]).split(' '))
  def Variable(self, variable):
    return RunCmd(['pkg-config', '--variable=%s' % variable, self.module])

class Buildable(object):
  """ Base class for buildable objects."""
  def __init__(self):
    self.output = ''
  def Build(self):
    """ Builds the object and returns the output file. """
    return ''

class CObj(Buildable):
  def __init__(self, src, output):
    self.src = src
    self.output = output
    self.cflags = ''
  def Build(self):
    if isinstance(self.src, Buildable):
      self.src = self.src.Build()
    if RequiresBuild(self.output, self.src):
      args = ['gcc', '-c']
      args.extend(self.cflags)
      args.extend(['-o', self.output, self.src])
      RunCmd(args)
      ReportBuilt(self.output, self.src)
    return self.output

class CCompiler(object):
  def __init__(self, cflags):
    self.cflags = cflags
  def Obj(self, src, output=None):
    if output is None:
      output = BuildPath('%s.o' % FileName(src))
    obj = CObj(src, output)
    obj.cflags = self.cflags
    return obj

class CResource(Buildable):
  def __init__(self, src, output):
    self.src = src
    self.output = output
  def Build(self):
    if isinstance(self.src, Buildable):
      self.src = self.src.Build()
    if RequiresBuild(self.output, self.src):
      self.RunCmd()
      ReportBuilt(self.output, self.src)
    return self.output
  def RunCmd(self):
    src = open(self.src, 'r')
    output = open(self.output, 'w')
    output.write("// Generated automatically from %s" % self.src)
    output.write(os.linesep)
    output.write("#include <glib.h>")
    output.write(os.linesep)
    output.write(os.linesep)
    output.write('gchar* %s = ""' % FileName(self.src))
    for line in src.readlines():
      line = line.rstrip()
      if line:
        output.write(os.linesep)
        output.write('"%s"' % line.replace('\\', '\\\\').replace('"', '\\"'))
    output.write(";")
    output.write(os.linesep)
    output.close()
    
class CResourceCompiler(object):
  def __init__(self):
    pass
  def Resource(self, src, output=None):
    if output is None:
      output = BuildPath('%s_res.c' % os.path.splitext(src)[0])
    res = CResource(src, output)
    return res

class SharedLib(Buildable):
  def __init__(self, name):
    self.objs = []
    self.output = BuildPath('%s.so' % name)
    self.libs = []
  def AddObj(self, obj):
    self.objs.append(obj)
  def Build(self):
    objs = []
    for obj in self.objs:
      objs.append(obj.Build())
    if RequiresBuild(*Extend([self.output], objs)):
      args = ['g++', '-shared', '-o', self.output]
      args.extend(self.libs)
      args.extend(objs)
      RunCmd(args)
      ReportBuilt(*Extend([self.output], objs))
    return self.output

class Executable(Buildable):
  def __init__(self, name):
    self.objs = []
    self.output = BuildPath('%s' % name)
    self.libs = []
  def AddObj(self, obj):
    self.objs.append(obj)
  def Build(self):
    objs = []
    for obj in self.objs:
      objs.append(obj.Build())
    if RequiresBuild(*Extend([self.output], objs)):
      args = ['g++', '-o', self.output]
      args.extend(self.libs)
      args.extend(objs)
      RunCmd(args)
      ReportBuilt(*Extend([self.output], objs))
    return self.output

class Copier(Buildable):
  def __init__(self, src, dst):
    self.src = src
    self.output = os.path.join(dst, BaseName(self.src))
  def Build(self):
    if isinstance(self.src, Buildable):
      self.src = self.src.Build()
    if RequiresBuild(self.output, self.src):
      RunCmd(['cp', self.src, self.output])
    return self.output

def NonEmpty(s):
  return len(s.strip()) > 0

def Extend(l1, l2):
  l1.extend(l2)
  return l1
