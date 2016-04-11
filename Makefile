CC=g++
CFLAGS=-c -std=c++11 -DGL_GLEXT_PROTOTYPES
INCLUDES=-I "src" \
         -I "/usr/include/freetype2" \
         -I "/usr/local/include/cef3" \
         -I "/usr/include/gtk-2.0"
LDFLAGS=-L "bin" -lpthread -lGL -lGLU -lSDL -lfreeimage \
        -lfreetype -lavutil -lavformat -lavcodec -lz -lavutil \
        -lm -lcurl -lao -lsqlite3 -lcef_dll_wrapper -lcef

SOURCES=ui/render.cc ui/img.cc ui/box.cc ui/animation.cc \
	      ui/texture.cc ui/fbo.cc ui/shader.cc ui/geom.cc ui/text.cc\
        ui/widgets/navigation.cc ui/widgets/widget.cc \
        ui/widgets/event.cc ui/widgets/clock.cc \
        ui/widgets/notification.cc ui/widgets/panel.cc \
        ui/widgets/button.cc ui/widgets/timeselector.cc \
        ui/widgets/mediaplayerui.cc ui/widgets/slider.cc \
        ui/widgets/keyboard.cc ui/widgets/list.cc \
        ui/panels/alarm_panel.cc ui/panels/entertainment_panel.cc \
        ui/panels/stock_panel.cc ui/panels/browser_panel.cc \
        ui/panels/settings_panel.cc ui/utils.cc \
        mediaplayer/mediaplayer.cc storage/storage.cc \
        network/mailer.cc network/json11.cc main.cc

OBJECTS=$(addprefix obj/,$(SOURCES:%.cc=%.o))
DEPS=$(OBJECTS:%.o=%.d)
EXECUTABLE=bin/kiosk
SHELL=/bin/bash

.PHONY: all clean cleanall deploy

all: debug-build

deploy: cleanall
	@bash scripts/deploy.sh

debug-build-prep:
	$(eval CFLAGS:=$(CFLAGS) -MMD -MP -DDEBUG)
	@echo -e "Debug build:"
	@echo -e "cflags: $(CFLAGS)"
	@echo -e "include: $(INCLUDES)"
	@echo -e "ldflags: $(LDFLAGS)"
debug-build: debug-build-prep $(OBJECTS)
	@cp -R cef3/64bit/* bin/
	@cp -R data bin/
	@echo "Linking debug build $(EXECUTABLE)"
	@$(CC) $(OBJECTS) -o $(EXECUTABLE) -L"bin" $(LDFLAGS)

release-build-prep:
	$(eval CFLAGS:=$(CFLAGS) -O2)
	@echo -e "Release build:"
	@echo -e "cflags: $(CFLAGS)"
	@echo -e "include: $(INCLUDES)"
	@echo -e "ldflags: $(LDFLAGS)"
release-build: release-build-prep $(OBJECTS)
	@cp -R cef3/32bit/* bin/
	@cp -R data bin/
	@echo "Linking release build $(EXECUTABLE)"
	@$(CC) $(OBJECTS) -o $(EXECUTABLE) -L"bin" $(LDFLAGS)

obj/%.o: %.cc
	@mkdir -p $(dir $@)
	@echo -e "Compiling" $(notdir $<)
	@$(CC) -g3 $(CFLAGS) $(INCLUDES) $< -o $@

-include $(DEPS)

clean:
	@echo "Removing object files and executable"
	@rm -R $(OBJECTS) 2>/dev/null || true
	@rm $(EXECUTABLE) 2>/dev/null || true

cleanall:
	@echo "Removing object folder content and binary folder content"
	@rm -R obj/* bin/* 2>/dev/null || true

