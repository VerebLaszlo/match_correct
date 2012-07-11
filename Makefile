# Makefile
####################################################################################################
# Autómatikus változók:												\
$^ az összes feltétel listája belértve a mappákat is				\
$< az első feltétel													\
$@ a cél															\
$? változtatott fájlok												\
$* ??????															\
#####################################################################
# ?= csak ha nincs definiálva										\
#####################################################################

include system.mk

DER=	# ha 'make DER=ok' ként hivom meg, akkor DER-nek "ok" lesz az értéke!!!!!

CC := gcc
CFLAGS := -std=gnu99 -O3 -ggdb3

include config.mk

errorFlags := -Wall -Wextra -Wformat-security -Wmissing-include-dirs -Wswitch-default
errorFlags += -Wstrict-prototypes -Wold-style-definition -Wno-aggregate-return

errorExtraFlags := -Wshadow -Winit-self -Wunsafe-loop-optimizations -Wcast-qual
errorExtraFlags += -Wcast-align -Wwrite-strings -Wlogical-op -Waggregate-return
errorExtraFlags += -Wmissing-prototypes -Wmissing-declarations -Wdisabled-optimization
errorExtraFlags += -Wconversion -Wswitch-enum

errorOptionalFlags := -Wformat-nonliteral -Wconversion -Wswitch-enum -Wbad-function-cast
errorOptionalFlags += -Wredundant-decls

CFLAGS += -march=$(shell arch) $(errorFlags)
srcdir := src
incdir := include
objdir := object_dir
objects := $(patsubst $(srcdir)/%.c,$(objdir)/%.o,$(wildcard $(srcdir)/*.c))
makes := $(patsubst $(srcdir)/%.c,$(objdir)/%.d,$(wildcard $(srcdir)/*.c))
testdir := test
objects += $(patsubst $(testdir)/%.c,$(objdir)/%.o,$(wildcard $(testdir)/*.c))
makes += $(patsubst $(testdir)/%.c,$(objdir)/%.d,$(wildcard $(testdir)/*.c))
includes := -I$(incdir) #-I/usr/include

objs_test := main_test.o signals.o detector.o binary_system.o binary_system_mass.o program_functions.o
objs_test += binary_system_spin.o util_math.o util_IO.o util.o test.o parameters.o lal_wrapper.o
objs_test += parser.o

vpath
vpath %.c $(srcdir)
vpath %.c $(testdir)
vpath %.h $(incdir)
vpath %.o $(objdir)
vpath %.d $(objdir)
# EZT MÉG LE KELL ELLENŐRIZNI
#vpath lib%.so $(subst -L,,$(subst lib\ -L,lib:,$(shell pkg-config --libs-only-L lalsimulation)))
#vpath lib%.a $(subst -L,,$(subst lib\ -L,lib:,$(shell pkg-config --libs-only-L lalsimulation)))

lal_includes := $(shell pkg-config --cflags lalsimulation) $(shell pkg-config --cflags libconfig)
includes += $(lal_includes)
lal_libraries := $(shell pkg-config --libs-only-l lalsimulation)
lal_libraries_path := $(shell pkg-config --libs-only-L lalsimulation) $(shell pkg-config --libs-only-L libconfig) -lmetaio

all : test Makefile

test main : CFLAGS += $(errorExtraFlags) $(lal_libraries_path)
test : macros += -DTEST

test main : $(objects) -lfftw3 -lm $(shell pkg-config --libs-only-l libconfig)
	@echo -e $(start)'Linking: $@'$(reset)
	$(hide_echo)$(CC) $(CFLAGS) $(macros) $(lal_libraries) -o $@ $^
	@echo -e $(end)'Finished linking: $@'$(reset)
	@echo ' '

$(objdir)/%.o : %.h

$(objdir)/%.o : %.c | $(objdir)
	@echo -e $(start)'Building file: $<'$(reset)
	$(hide_echo)$(CC) $(CFLAGS) $(CPPFLAGS) $(includes) $(macros) $(lal_libraries) -c -MMD -MF$(@:%.o=%.d) -MT$(@:%.o=%.d) $< -o $@
	@echo -e $(end)'Finished building: $<'$(reset)
	@echo ' '

# kisegítők
#$(objdir)/%.o: | $(objdir)		# NEM MŰKÖDIK!!!!!!!!!!!!!!!!!!!!!!!!!
#	@echo 'obj'

#$(objdir)/%.d: | $(objdir)		# NEM MŰKÖDIK!!!!!!!!!!!!!!!!!!!!!!!!!
#	@echo 'makes'

$(objdir) :
	mkdir $(objdir)

print : %.c					# kilistázza a változtatott fájlokat az utolsó print óta
	lpr -p $?
	touch $@

debug : 

release : CFLAGS += -O3

# parancsok
.PHONY : doxy cleandoxy clean cleanobj cleanall all # csak utasítás név, nem cél

doxy : doxyutil
	doxygen Doxyfile$(TEST)

doxyutil :
	-mkdir doc
	-mkdir doc/html
	-mkdir doc/latex
	-rm src/*.h
	cp doc/style.sty doc/html/style.sty
	cp doc/style.sty doc/latex/style.sty
	clear scr

cleandoxy :
	-rm -R doc/*

clean : cleanobj

cleanall : cleanobj
	-rm $(objdir)/*.d
	-rm main test
	clear

cleanobj :
	-rm $(objdir)/*.o
	clear
