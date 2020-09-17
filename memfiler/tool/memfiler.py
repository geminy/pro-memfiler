#####################################
# todo with addr2line and libunwind #
#####################################

#!/usr/bin/env python


from optparse import OptionParser
import re
import os
import sys
import threading
import subprocess
from time import sleep


class DemangleThread(threading.Thread):
    def __init__(self, absaddress, name, libpath):
        threading.Thread.__init__(self)
        self.absaddrlist = absaddress
        self.name = name
        self.libpath = libpath
        self.funcnamelist = []

    def run(self):
        addrtoline = []
        for index in range(0, len(self.absaddrlist)):
            if index % 100000 == 0 and index != 0:
                self.__analysis__(addrtoline)
                addrtoline = []
                addrtoline.append(str(hex(self.absaddrlist[index])))
        self.__analysis__(addrtoline)

    def __analysis__(self, addr):
        print "a"
        addrtoline = ["addr2line", "-a", "-C", "-f"]
        tmp = ["-e", self.libpath]
        addrtoline += addr
        addrtoline += tmp
        process = subprocess.Popen(addrtoline, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
        (stdout, stderr) = process.communicate()
        process.poll()
        stdout = stdout.strip()
        outline = stdout.split("\n")
        for index in range(0, len(outline), 3):
            tmp = []
            tmp.append(outline[index])
            tmp.append(outline[index + 1])
            tmp.append(outline[index + 2])
            self.funcnamelist.append(tmp)


####################


class Demangler:
    def __init__(self):
        self.malloclist = []
        self.addressdict = {}
        self.mapinfolist = []

    def demangle(self, mp, symbolpath):
        print "demangling..."
        assert mp
        assert os.path.exists(symbolpath)
        self._demangleaddress(mp, symbolpath)
        for minfo in mp.malloclist:
            dminfo = MallocInfo()
            dminfo.count = minfo.count
            dminfo.size = minfo.size
            for bt in minfo.backtrace:
                if self.addressdict.has_key(bt):
                    dminfo.backtrace.append(self.addressdict[bt])
                else:
                    dminfo.backtrace.append([bt, ["", "", ""], ""])
            self.malloclist.append(dminfo)

    def _demangleaddress(self, mp, symbolpath):
        map2absaddress = {}
        map2address = {}
        map2libpath = {}
        threaddict = {}
        addresslist = mp.addresslist
        mapinfolist = mp.mapinfolist

        for addr in addresslist:
            preminfo = None
            for minfo in mapinfolist:
                if int(addr, 16) < int(minfo.start, 16):
                    if preminfo is None:
                        break
                    absaddr = 0x0
                    if int(preminfo.start, 16) == 0x8000:
                        absaddr = int(addr, 16)
                    else:
                        absaddr = int(addr, 16) - int(preminfo.start, 16)
                    # todo
                    # if os.path.isabs(preminfo.name):
                    #     libpath = preminfo.name
                    # else:
                    #     libpath = symbolpath + preminfo.name
                    libpath = symbolpath + preminfo.name
                    if not os.path.exists(libpath):
                        libpath = None
                    if not libpath is None:
                        map2absaddress.setdefault(preminfo.name, []).append(absaddr)
                        map2address.setdefault(preminfo.name, []).append(addr)
                        map2libpath[preminfo.name] = libpath
                        break
                else:
                    preminfo = minfo

        for key, value in map2absaddress.iteritems():
            thread = DemangleThread(value, key, map2libpath[key])
            thread.start()
            threaddict[key] = thread

        for key, value in threaddict.iteritems():
            value.join()
            for index in range(len(map2address[key])):
                if index >= len(value.funcnamelist):
                    break
                else:
                    self.addressdict[map2address[key][index]] = [map2address[key][index], value.funcnamelist[index], value.name]

    def outputresult(self, filename, showfile):
        print "output...\n\t%s" % (filename)
        fd = open(filename, "w")
        for info in self.malloclist:
            sectionheader = "size:" + str(info.size) + " " + "count:" + str(info.count)
            fd.write(sectionheader + "\n")
            for bt in info.backtrace:
                if showfile:
                    fd.write("\t" + str(bt[0]) + "\t" + bt[1][1] + "\t" + bt[2] + "+" + bt[1][0] + "\t" + bt[1][2] + "\n")
                else:
                    fd.write("\t" + str(bt[0]) + "\t" + bt[1][1] + "\t" + bt[2] + "+" + bt[1][0] + "\n")
            fd.write("\n")
        fd.close()


####################


class MapInfo:
    MI_LINEMATCH = "([0-9a-f]+)-([0-9a-f]+) +r-xp +[0-9]+ [0-9a-f]+:[0-9a-f]+ [0-9]+ +(.+)"

    def __init__(self):
        self.start = ""
        self.end = ""
        self.name = ""


####################


class MallocInfo:
    def __init__(self):
        self.count = 0
        self.size = 0
        self.backtrace = []


####################


class MallocInfoList:
    def __init__(self):
        self.mallocdict = {}

    def parsemallocinfo(self, line):
        btlist = line.split()
        size = len(btlist)
        checksum = 0
        for bt in btlist:
            checksum ^= (int(bt, 16))
        self._addmallocinfo(size, btlist, checksum)

    def getmallocinfolist(self, malloclist, addresslist):
        tmp = []
        for infokey, infovalue in self.mallocdict.items():
            for sizekey, sizevalue in infovalue.items():
                for checkkey, checkvalue in sizevalue.items():
                    for item in checkvalue:
                        mi = MallocInfo()
                        mi.count = item[0]
                        mi.size = infokey
                        mi.backtrace = item[1]
                        malloclist.append(mi)
                        tmp += mi.backtrace
        tmp = list(set(tmp))
        addresslist[:] = tmp[:]

    def _addmallocinfo(self, size, btlist, checksum):
        info = self.mallocdict.get(size, None)
        if not info is None:
            sizevalue = info.get(size, None)
            if not sizevalue is None:
                checkvalue = sizevalue.get(checksum, None)
                if not checkvalue is None:
                    recorded = False
                    for item in checkvalue:
                        if self._cmpbtlist(item[1], btlist):
                            item[0] += 1
                            recorded = True
                            break
                    if not recorded:
                        checkvalue.append([1, btlist])
                else:
                    sizevalue[checksum] = [[1, btlist]]
            else:
                info[size] = {checksum: [[1, btlist]]}
        else:
            self.mallocdict[size] = {size: {checksum: [[1, btlist]]}}

    def _cmpbtlist(self, firstitem, seconditem):
        for i in range(0, len(firstitem)):
            if firstitem[i] != seconditem[i]:
                return False
        return True


####################


class MallocParser:
    def __init__(self, symbolpath):
        self.addresscount = -1
        self.malloclist = []
        self.addresslist = []
        self.mapinfolist = []
        self.mapspattern = re.compile(MapInfo.MI_LINEMATCH)

    def parsemalloc(self, filepath):
        print "malloc parsing...\n\t%s" % (filepath)
        fd = open(filepath)
        line = fd.readline().strip()
        assert Memfiler.MF_BAD == line
        line = fd.readline().strip()
        self.addresscount = int(line)
        print "\t%s %d" % (Memfiler.MF_BAD, self.addresscount)
        mallocinfolist = MallocInfoList()
        while True:
            line = fd.readline().strip()
            if Memfiler.MF_END == line:
                break
            print "\t%s" % (line)
            mallocinfolist.parsemallocinfo(line)
        fd.close()
        mallocinfolist.getmallocinfolist(self.malloclist, self.addresslist)
        for malloc in self.malloclist:
            print "\t%d %d" % (malloc.count, malloc.size),
            for bt in malloc.backtrace:
                print bt,
            print
        print "\t%s done" % (filepath)

    def parsemaps(self, filepath):
        print "maps parsing...\n\t%s" % (filepath)
        fd = open(filepath)
        line = fd.readline().strip()
        assert Memfiler.MF_MAPS == line
        while True:
            line = fd.readline().strip()
            if Memfiler.MF_END == line:
                break
            # print "\t%s" % (line)
            self._parsemapsline(line)
        fd.close()
        self._dumpmapsinfo()
        print "\t%s done" % (filepath)

    def _parsemapsline(self, line):
        maps = self.mapspattern.match(line)
        if not maps is None:
            mapinfo = MapInfo()
            mapinfo.start = maps.group(1)
            mapinfo.end = maps.group(2)
            mapinfo.name = maps.group(3)
            self.mapinfolist.append(mapinfo)

    def _dumpmapsinfo(self):
        for mapinfo in self.mapinfolist:
            print "\t%s %s %s" % (mapinfo.start, mapinfo.end, mapinfo.name)


####################


class Memfiler(threading.Thread):
    MF_OBJECTS = ("mf_info.log",
                  "mf_backtrace.log",
                  "mf_bad.log",
                  "mf_maps.log",
                  "mf_smaps.log")
    MF_INFO = "INFO"
    MF_BACKTRACE = "BACKTRACE"
    MF_BAD = "BAD"
    MF_MAPS = "MAPS"
    MF_SMAPS = "SMAPS"
    MF_END = "END"

    def __init__(self, inputpath, symbolpath, outputpath):
        threading.Thread.__init__(self)
        self.inputpath = inputpath
        self.symbolpath = symbolpath
        self.outputpath = outputpath
        self.badfile = None
        self.mapsfile = None

    def run(self):
        # just for test
        for f in self.inputpath:
            if os.path.basename(f) == "mf_bad.log":
                self.badfile = f
            elif os.path.basename(f) == "mf_maps.log":
                self.mapsfile = f
        if (self.badfile is None) or (self.mapsfile is None):
            print "mf_bad.log and mf_maps.log are both required!"
            exit()

        if self.outputpath is None:
            self.outputpath = os.getcwd()

        mp = MallocParser(self.symbolpath)
        mp.parsemalloc(self.badfile)
        mp.parsemaps(self.mapsfile)

        dm = Demangler()
        dm.demangle(mp, self.symbolpath)
        dm.outputresult(self.outputpath + os.path.sep + os.path.basename(self.badfile) + ".dm", True)


####################


def main():
    parser = OptionParser()
    parser.add_option("-i", "--input", dest="inputpath", help="input path.")
    parser.add_option("-s", "--symbol", dest="symbolpath", help="root path of symbol file.")
    parser.add_option("-o", "--output", dest="outputpath", help="path for output, default is current work dir!")

    (opts, args) = parser.parse_args()
    if (opts.inputpath is None) or (opts.symbolpath is None):
        print "Invalid argument!"
        exit()
    if (not os.path.exists(opts.inputpath)) or (not os.path.exists(opts.symbolpath)):
        print "Invalid path!"
        exit()

    pathlist = []
    if os.path.isfile(opts.inputpath) and (os.path.basename(opts.inputpath) in Memfiler.MF_OBJECTS):
        pathlist.append(os.path.abspath(opts.inputpath))
    elif os.path.isdir(opts.inputpath):
        for f in os.listdir(opts.inputpath):
            if (os.path.isfile(os.path.join(opts.inputpath, f)) and (f in Memfiler.MF_OBJECTS)):
                pathlist.append(os.path.abspath(opts.inputpath) + os.path.sep + f)

    if len(pathlist) < 1:
        print "No valid file found from input path!"
        exit()
    print "objects:"
    for p in pathlist:
        print "\t%s" % (p)
    sleep(1)

    t = Memfiler(pathlist, opts.symbolpath, opts.outputpath)
    t.start()
    t.join()

    # threadlist = []
    # for p in pathlist:
    #     thread = Memfiler(p, opts.symbolpath, opts.outputpath)
    #     thread.start()
    #     threadlist.append(thread)
    #
    # for t in threadlist:
    #     t.join()


####################


if __name__ == "__main__":
    main()
