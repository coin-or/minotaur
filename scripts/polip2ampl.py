#!/usr/bin/python
import fileinput
import re
import sys
import types

INF=1e20

class Program:
	UNDEFINED=0
	OBJECTIVE=1
	CONSTRAINTS=2
	COLON=3
	BOUNDS=4
	GENERAL=5
	BINARY=6
	END=7
	
class PState:
	NOTHING=0
	CONSTANT=1
	VAR=2
	MULT=3
	CARET=4
	PLUS=7
	MINUS=8
	END=9

class Polynomial:
	terms=[]
	def append(self,mo):
		self.terms.append(mo)
	def write(self):
		for mm in self.terms:
			mm.write()

class Monomial:
	coeff=0.0
	terms={}
	def write(self):
		s=""
		if (self.coeff>=0.0):
			s+="+ "+str(self.coeff)
		else:
			s+="- "+str(abs(self.coeff))
		for t in self.terms.keys():
			s+="*"
			s+=massage(t)
			s+="^"+str(self.terms[t])
		print s,


class ObjType:
	UNDEFINED=0
	MIN=1
	MAX=2

class Variable:
	name=""
	lb=-INF
	ub=INF
	typ=""
	def write(self):
		#print var,lb,ub
		print "var", massage(self.name),
		if (self.lb>-INF):
			print ">=", self.lb,
			if (self.ub<INF):
				print ",",
		if (self.ub<INF):
			print "<=", self.ub,
			if (self.typ=="I" or self.typ=="B"):
				print ",",
		if (self.typ=="I"):
			print "integer",
		elif (self.typ=="B"):
			print "binary",
		print ";"

class Constraint:
	name=""
	rhs=""
	poly=Polynomial()
	def write(self):
		print massage(self.name.lower()), 
		self.poly.write()
		print self.rhs, ";"


class Objective:
	name=""
	sense="MIN"
	poly=Polynomial()
	def write(self):
		print massage(self.name.lower()),
		self.poly.write()
		print ";"

class Problem:
	obj=Objective()
	pvars={}
	pcons=[]
	def write(self):
		for v in self.pvars.keys():
			self.pvars[v].write()
		print ''
		print ''
		self.obj.write()
		for c in self.pcons:
			print ''
			c.write()

def reportErr(s):
	print 'error: ', s
	exit(123213)

def massage(t):
	s=t.replace('!','__e__')
	s=s.replace('#','__h__')
	s=s.replace('$','__d__')
	s=s.replace('%','__p__')
	s=s.replace('&','__a__')
	s=s.replace(';','__s__')
	s=s.replace('?','__q__')
	s=s.replace('@','__e__')
	return s





def removeComment(line):
	l=re.search('^[^\\\\]*',line)
	l1=l.group(0)
	l2=re.search('^[^\n\r\f]*',l1)
	return l2.group(0)

def checkProg(line):
	has=0
	l0=re.search(':',line,flags=re.IGNORECASE)
	if (type(l0) is not types.NoneType):
		has=1 
	l0=re.search('^[ \t\n\r\f\v]*minimize|min|maximize|max',line,flags=re.IGNORECASE)
	if (type(l0) is not types.NoneType):
		return Program.OBJECTIVE, has
	l0=re.search('^[ \t\n\r\f\v]*subject\ to|st|s\.t\.',line,flags=re.IGNORECASE)
	if (type(l0) is not types.NoneType):
		return Program.CONSTRAINTS, has
	l0=re.search('^[ \t\n\r\f\v]*bounds',line,flags=re.IGNORECASE)
	if (type(l0) is not types.NoneType):
		return Program.BOUNDS, has
	l0=re.search('^[ \t\n\r\f\v]*general',line,flags=re.IGNORECASE)
	if (type(l0) is not types.NoneType):
		return Program.GENERAL, has
	l0=re.search('^[ \t\n\r\f\v]*binary',line,flags=re.IGNORECASE)
	if (type(l0) is not types.NoneType):
		return Program.BINARY, has
	l0=re.search('^[ \t\n\r\f\v]*end',line,flags=re.IGNORECASE)
	if (type(l0) is not types.NoneType):
		return Program.END, has
	return Program.UNDEFINED, has



def removeNext(b):
	# find leading spaces
	l0 = re.search('^ +',b)
	if (type(l0) is not types.NoneType):
		l1 = l0.group(0)
		b1 = b[len(l1):len(b)]
		return b1, l1, PState.NOTHING 
	if (b[0]=='~'):
		return '','',PState.END
	if (b[0]=='-'):
		b1 = b[1:len(b)]
		return b1, '-', PState.MINUS 
	if (b[0]=='+'):
		b1 = b[1:len(b)]
		return b1, '+', PState.PLUS 
	if (b[0]=='*'):
		b1 = b[1:len(b)]
		return b1, '*', PState.MULT 
	if (b[0]=='^'):
		b1 = b[1:len(b)]
		return b1,'^',PState.CARET

	l0 = re.search('^[a-z]',b,flags=re.IGNORECASE)
	if (type(l0) is not types.NoneType):
		l1 = re.search('^[a-z0-9\#]+',b,flags=re.IGNORECASE).group(0)
		b1 = b[len(l1):len(b)]
		return b1, l1, PState.VAR 

	l0 = re.search('^[0-9]+\.?[0-9]*',b)
	if (type(l0) is not types.NoneType):
		l1 = l0.group(0)
		b1 = b[len(l1):len(b)]
		return b1, l1, PState.CONSTANT 
	return b, '', PState.NOTHING 


def readPoly(a,pr):
	b = a+'*1.0'+'~'
	pt = PState.NOTHING
	p = Polynomial()
	p.terms=[]
	m = Monomial()
	m.coeff=-INF
	m.terms={}
	numsides=0
	lastvar=''
	lastconst = -INF
	nex=''
	while (len(b)>0):
		#p.write()
		#print ''
		#for mm in p:
		#	mm.write()
		#print ';'
		#print 'm = ', 
		#m.write()
		#print ';'
		#print 'lastvar = ', lastvar, 'lastconst = ', lastconst

		b,nex,nt=removeNext(b)
		if (nt == PState.NOTHING):
			#print 'space', nex
			# no nothing
			nothing=0
		elif (nt == PState.END):
			if (m.coeff != -INF):
				p.append(m)
				m = Monomial()
				m.coeff=-INF
				m.terms={}
		elif (nt == PState.CONSTANT):
			#print 'constant', nex
			if (pt==PState.NOTHING):
				m.coeff=-INF
				m.terms={}
				lastvar=''
				lastconst=float(nex)
			elif (pt==PState.CONSTANT):
				reportErr("constant after constant")
				lastconst=float(nex)
			elif (pt==PState.VAR):
				lastvar=nex
				lastconst=float(nex)
			elif (pt==PState.MULT):
				lastconst=float(nex)
			elif (pt==PState.PLUS):
				lastconst=float(nex)
				m.coeff = 1.0
			elif (pt==PState.MINUS):
				lastconst=-1.0*float(nex)
			elif (pt==PState.CARET):
				if (lastvar in m.terms):
					m.terms[lastvar]+=float(nex)
				else:
					m.terms[lastvar]=float(nex)
				lastconst=-INF

			else:
				reportErr("unknown state")
			pt=nt

		elif (nt == PState.VAR):
			#print 'var', nex, pt
			try:
				v2=pr.pvars[nex]
			except KeyError:
				v2=Variable()
				v2.name=nex
				v2.lb=-INF
				v2.ub=INF
				v2.typ="C"
				pr.pvars[nex]=v2

			if (pt==PState.NOTHING):
				lastvar=nex
				m.coeff=1.0
			elif (pt==PState.CONSTANT):
				if (m.coeff==-INF):
					m.coeff = lastconst
				else:
					m.coeff *= lastconst
				lastvar=nex
			elif (pt==PState.VAR):
				if (lastvar in m.terms):
					m.terms[lastvar]+=1
				else:
					m.terms[lastvar]=1
				lastvar=nex
			elif (pt==PState.MULT):
				lastvar=nex
			elif (pt==PState.PLUS):
				if (m.coeff!=-INF):
					p.append(m)
					m = Monomial()
					m.coeff=-INF
					m.terms={}
				m.coeff=1.0
				lastvar=nex
			elif (pt==PState.MINUS):
				if (m.coeff!=-INF):
					p.append(m)
					m = Monomial()
					m.coeff=-INF
					m.terms={}
				m.coeff=-1.0
				lastvar=nex
			elif (pt==PState.CARET):
				reportErr("var after caret")
			else:
				reportErr("unknown state")
			pt=nt

		elif (nt == PState.MULT):
			#print '*', nex
			if (pt==PState.NOTHING):
				reportErr("* after nothing")
			elif (pt==PState.CONSTANT):
				if (m.coeff==-INF and lastconst != -INF):
					m.coeff  = lastconst
				elif (m.coeff==-INF and lastconst==-INF):
					reporterr("101")
				elif (m.coeff!=-INF and lastconst!=-INF):
					m.coeff *= lastconst
				elif (m.coeff!=-INF and lastconst==-INF):
					# do nothing
					pt=nt
			elif (pt==PState.VAR):
				if (lastvar in m.terms):
					m.terms[lastvar]+=1
				else:
					m.terms[lastvar]=1
				lastvar=nex
			elif (pt==PState.MULT):
				reportErr("* after *")
			elif (pt==PState.PLUS):
				reportErr("+*")
			elif (pt==PState.MINUS):
				reportErr("-*")
			elif (pt==PState.CARET):
				reportErr("^*")
			else:
				reportErr("unknown state")
			pt=nt
		elif (nt == PState.CARET):
			#print '^', nex
			if (pt==PState.NOTHING):
				reportErr("^ after nothing")
			elif (pt==PState.CONSTANT):
				reportErr("^ after constant")
			elif (pt==PState.VAR):
				pt=nt
				if (m.coeff==-INF):
					m.coeff=1.0
			elif (pt==PState.MULT):
				reportErr("*^")
			elif (pt==PState.PLUS):
				reportErr("+^")
			elif (pt==PState.MINUS):
				reportErr("-^")
			elif (pt==PState.CARET):
				reportErr("^^")
			else:
				reportErr("unknown state")
			pt=nt
		elif (nt == PState.PLUS or nt == PState.MINUS):
			#print '+/-', nex
			if (pt==PState.NOTHING):
				if (nt == PState.PLUS):
					lastconst=1.0
				else:
					lastconst=-1.0
				lastvar=''
				m = Monomial()
				m.coeff=-INF
				m.terms={}
			elif (pt==PState.CONSTANT):
				if (lastconst!=-INF):
					m.coeff = lastconst
				p.append(m)
				m = Monomial()
				m.coeff=-INF
				m.terms={}
				lastvar=''
				if (nt == PState.PLUS):
					lastconst=1
				else:
					lastconst=-1
				m = Monomial()
				m.coeff=-INF
				m.terms={}
			elif (pt==PState.VAR):
				if (lastvar in m.terms):
					m.terms[lastvar]+=1
				else:
					m.terms[lastvar]=1
				p.append(m)
				m = Monomial()
				m.coeff=-INF
				m.terms={}
				if (nt == PState.PLUS):
					lastconst=1
				else:
					lastconst=-1
				m.coeff=-INF
				m.terms={}
			elif (pt==PState.MULT):
				reportErr("*+")
			elif (pt==PState.PLUS):
				reportErr("++")
			elif (pt==PState.MINUS):
				reportErr("-+")
			elif (pt==PState.CARET):
				reportErr("^+")
			else:
				reportErr("unknown state")
			pt=nt
		else:
			sys.exit(23423)
		
	return p,pr


def doObj(pr,b):
	#print "doing obj"
	#print 'b = ',b
	objtype = ObjType.UNDEFINED
	l2=re.search('^[ \t\n\r\f\v]*minimize|min',b,flags=re.IGNORECASE)
	l4=re.search('^[ \t\n\r\f\v]*maximize|max',b,flags=re.IGNORECASE)
	if (type(l2) is not types.NoneType):
		objtype = ObjType.MIN
	elif (type(l4) is not types.NoneType):
		objtype = ObjType.MAX
	else:
		objtype = ObjType.UNDEFINED
		sys.exit(101)
	
	# remove everything before ':'
	l0 = re.search('(?<=:).*',b).group(0)
	poly,pr=readPoly(l0,pr)
	pr.obj.name = re.search('.*:',b).group(0)
	pr.obj.poly = poly
	#print pr.obj.objname,
	#poly.write()
	#print ";"
	return pr

def doCons(pr,b):
	#print "doing cons"
	#print b
	c = Constraint()
	c.name=re.search('.*:',b).group(0)
	c.rhs=re.search('[<=>]+.*',b).group(0)
	# remove everything before ':'
	l0 = re.search('(?<=:).*',b).group(0)
	# remove everything after >,<,= 
	l0 = re.search('[^<>=]*',l0).group(0)
	poly,pr=readPoly(l0,pr)

	c.poly = poly
	#print c.name,
	#c.poly.write()
	#print c.rhs,";"
	#reportErr("103")
	pr.pcons.append(c)
	return pr

def getVBound(vstr):
	#print vstr
	lb=-INF
	ub=INF
	var=""
	vals=[]
	syms=[]
	while (len(vstr)>0):
		wrd=vstr.pop()
		#print wrd
		if (wrd=="<=" or wrd=="=<" or wrd=="<"):
			syms.append('L')
		elif (wrd==">=" or wrd=="=>" or wrd==">"):
			syms.append('G')
			++nsym
		elif (wrd=="=" or wrd=="=="):
			syms.append('E')
		else:
			vals.append(wrd)
		
	#print 'vals =', vals
	#print 'syms =', syms
	if (len(vals)!=len(syms)+1):
		reportErr("in getVBound, number of inequalities is not"
				+" same as numbers")
	i=0
	while (i<len(vals)):
		f=3.1414141
		if (vals[i].lower()=="-inf"):
			vals[i]=str(-INF)
		elif (vals[i].lower()=="inf"):
			vals[i]=str(INF)
		try:
			f=float(vals[i])
			if (i>=2):
				j=1
			else:
				j=0
			if (i>0):
				if (syms[j]=='L'):
					ub=f
				elif (syms[j]=='G'):
					lb=f
				elif (syms[j]=='E'):
					ub=f
					lb=f
			else: 
				if (syms[j]=='L'):
					lb=f
				elif (syms[j]=='G'):
					ub=f
				elif (syms[j]=='E'):
					ub=f
					lb=f
		except ValueError:
			if (var==""):
				var=vals[i]
			else:
				reportErr("two vars in line in bounds section")
		i+=1

	#print var, lb, ub
	return lb,ub,var


def doBounds(pr,b):
	#print "doing bounds"
	#print b
	b2=b.replace('<=','<<')
	b2=b2.replace('>>','>>')
	b2=b2.replace('==','!!!')
	b2=b2.replace('=',' = ')
	b2=b2.replace('!!!',' == ')
	b2=b2.replace('>>',' >= ')
	b2=b2.replace('<<',' <= ')
	isreal=0
	# remove space
	l0 = re.search('^ *bounds',b2,flags=re.IGNORECASE)
	if (type(l0) is not types.NoneType):
		b2 = b2[len(l0.group(0)):len(b2)]
 	s=re.split('[ \t\n\r\f\v:\*]+', b2)
	stlen=0
	lSt = False
	vstr=[]
	# first remove free vars
	i=0
	while (i<len(s)):
		if (s[i].lower()=="free"):
			del s[i]
			v2=Variable()
			try:
				v2=pr.pvars[s[i-i]]
			except KeyError:
				v2.name=s[i-1]
				v2.lb=-INF
				v2.ub=INF
				v2.typ="C"
				pr.pvars[s[i-1]]=v2
			del s[i-1]
		else:
			i+=1
	while (len(s)>0):
		wrd=s.pop()
		#print 'wrd =',wrd, 'vstr =', vstr
		if (len(wrd)==0):
			continue
		l0 = re.search('[<=>]',wrd)
		if (type(l0) is types.NoneType):
			# string
			if (lSt==True):
				lb,ub,var=getVBound(vstr)
				v2=Variable()
				try:
					v2=pr.pvars[var]
					v2.lb=lb
					v2.ub=ub
				except KeyError:
					v2.name=var
					v2.lb=lb
					v2.ub=ub
					v2.typ="C"
					pr.pvars[var]=v2
				#print "2 strings"
				vstr=[]
				vstr.append(wrd)
			else:
				vstr.append(wrd)
			lSt=True

		else:
			#symbol
			#print "symbol"
			if (lSt==False):
				print "2 symbols"
			vstr.append(wrd)
			lSt=False

	if (len(vstr)>0):
		lb,ub,var=getVBound(vstr)
		v2=Variable()
		try:
			v2=pr.pvars[var]
			v2.lb=lb
			v2.ub=ub
		except KeyError:
			v2.name=var
			v2.lb=lb
			v2.ub=ub
			v2.typ="C"
			pr.pvars[var]=v2
	return pr


def doTypes(pr,b,g):
	b2=re.search('^ *binary(.*)',b,flags=re.IGNORECASE)
	if (type(b2) is not types.NoneType):
		s=re.split('[ \t\n\r\f\v:\*]+', b2.group(1))
		for  wrd in s :
			if (wrd==""):
				continue
			try:
				v2=pr.pvars[wrd]
				if (v2.lb<0):
					v2.lb=0
				if (v2.ub>1):
					v2.ub=1
				v2.typ="B"
			except KeyError:
				v2=Variable()
				v2.name=wrd
				v2.lb=0
				v2.ub=1
				v2.typ="B"
				pr.pvars[wrd]=v2

		
	b2=re.search('^ *general(.*)',g,flags=re.IGNORECASE)
	if (type(b2) is not types.NoneType):
		s=re.split('[ \t\n\r\f\v:\*]+', b2.group(1))
		for  wrd in s :
			if (wrd==""):
				continue
			try:
				v2=pr.pvars[wrd]
				v2.typ="I"
			except KeyError:
				v2=Variable()
				v2.name=wrd
				v2.lb=-INF
				v2.ub=INF
				v2.typ="I"
				pr.pvars[wrd]=v2
		
	return pr


########## main ###########
i = 1
pipname=""
if(len(sys.argv)<2):
	reportErr("usage: ./pip2ampl filename.pip")

pipname=sys.argv[1]

print '#',pipname
stage=Program.UNDEFINED

fil=open(pipname,'r')

pr = Problem()
ptype = Program.UNDEFINED
a=fil.readline()
stphase=0
bnds=""
bndphase=0
binphase=0
genphase=0
bins=""
gens=""
while (a):
	b=removeComment(a)
	numco=0
	ptype, numco = checkProg(b)
	if (ptype == Program.END): 
		break
	elif (ptype == Program.BOUNDS):
		#print "doing bounds"
		a = fil.readline()
		bnds+=" "+b
		bndphase=1
		binphase=0
		genphase=0
	elif (ptype == Program.BINARY):
		#print "doing binary"
		a = fil.readline()
		bndphase=0
		binphase=1
		genphase=0
		bins+=" "+b
	elif (ptype == Program.GENERAL):
		a = fil.readline()
		bndphase=0
		binphase=0
		genphase=1
		gens+=" "+b
	elif (bndphase==1):
		bnds+=" "+b
		#print bnds
		a = fil.readline()
	elif (binphase==1):
		bins+=" "+b
		#print bins
		a = fil.readline()
	elif (genphase==1):
		gens+=" "+b
		a = fil.readline()
	elif (ptype == Program.OBJECTIVE or ptype==Program.CONSTRAINTS or
			numco==1):
		if (ptype==Program.OBJECTIVE):
			numco=2
			stphase=0
		elif (ptype==Program.CONSTRAINTS):
			stphase=2
		ptype2 = Program.UNDEFINED
		b2=''
		numco2=0
		bndphase=0
		binphase=0
		genphase=0
		#print a, ptype2
		while (a and ptype2 == Program.UNDEFINED):
			b+=" "+b2
			numco+=numco2
			a=fil.readline()
			if (a):
				b2=removeComment(a)
				ptype2,numco2=checkProg(b2)
			else:
				ptype2=Program.UNDEFINED
			if (stphase==1 and numco2==1):
				ptype2=Program.CONSTRAINTS
			elif (stphase==2 and numco+numco2==2):
				ptype2=Program.CONSTRAINTS
				stphase=1
			#print 'b= ',b
			#print 'b2= ',b2
			#print numco, numco2, ptype, ptype2, stphase

		if (ptype==Program.OBJECTIVE):
			pr=doObj(pr,b)
		elif (ptype==Program.CONSTRAINTS or (ptype==Program.UNDEFINED
				and stphase==1)):
			pr=doCons(pr,b)
	else:
		#print "doing nothing"
		#print a
		a = fil.readline()
fil.close()
pr=doBounds(pr,bnds)
pr=doTypes(pr,bins,gens)
pr.write()

