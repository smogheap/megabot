from pygame import *
from gloss import *
import random
import time
import optparse
try:
    import cwiid
except:
    pass


class Megabot(GlossGame):
    MODE_TITLE = "title"
    MODE_LOAD = "load"
    MODE_CONFIG = "config"
    MODE_GAME = "game"
    MODE_QUIT = "quit"

    FADE_TIME = 500
    SCREEN_RATIO = 256 / 240.0

    TITLE_COLOR = Color(1, 1, 1)

    def preload_content(self):
        global options

        ratio = Gloss.screen_resolution[0] / float(Gloss.screen_resolution[1])
        if ratio > Megabot.SCREEN_RATIO:
            height = Gloss.screen_resolution[1]
            width = int(height * Megabot.SCREEN_RATIO)
            if options.offset[0] is not None:
                left = options.offset[0]
                right = Gloss.screen_resolution[0] - width - left
            else:
                left = right = (Gloss.screen_resolution[0] - width) / 2
            self.display = {"size": (width, height), "pos": (left, 0)}
            self.displayMask = [{"size": (left, height), "pos": (0, 0)},
                                {"size": (right + 2, height),
                                 "pos": (left + width, 0)}]
        else:
            width = Gloss.screen_resolution[0]
            height = int(width / Megabot.SCREEN_RATIO)
            if options.offset[1] is not None:
                top = options.offset[1]
                bot = Gloss.screen_resolution[1] - height - top
            else:
                top = bot = (Gloss.screen_resolution[1] - height) / 2
            self.display = {"size": (width, height), "pos": (0, top)}
            self.displayMask = [{"size": (width, top), "pos": (0, 0)},
                                {"size": (width, bot + 2),
                                 "pos": (0, top + height)}]
        # the +2 on the right/bottom mask is for weird-resolution rounding

        self.scale = self.display["size"][1] / 240.0

        # load font
        # start at 60pt just as a guess
#        self.font = SpriteFont("asset/FreeMonoBold.ttf", 60)
        self.font = SpriteFont("asset/narpassword-fixed.ttf", 60)
        textsize = self.font.measure_string("0")
        textscale = (self.display["size"][0] / 40.0)/ textsize[0]
        # reload at 40-column, whatever that point size is
#        self.font = SpriteFont("asset/FreeMonoBold.ttf",
        self.font = SpriteFont("asset/narpassword-fixed.ttf",
                               int(60 * textscale))
        self.fontOffset = self.font.measure_string("0")[0] / 8

    def draw_loading_screen(self):
        #todo: generic text/menu system
        #todo: clean sizing system
        text = "LOADING..."
        size = self.font.measure_string(text)
        pos = Point.add(self.display["pos"], self.display["size"])
        pos = Point.subtract(pos, size)
        pos = Point.subtract(pos, self.font.measure_string("0"))
        pos = Point.subtract(pos, self.font.measure_string("0"))

        Gloss.draw_box(self.display["pos"], color = Color.BLACK,
                       width = self.display["size"][0],
                       height = self.display["size"][1])
        self.drawText(text, pos)
        self.drawMasks()

    def load_content(self):
        #todo: bontz sprite class
        #self.sprites[name][direction][anim][frame] ?
        self.title = Texture("asset/title.png")

        self.megabot = {}
        self.megabot["idle"] = []
        self.megabot["idle"].append(Texture("asset/megabot/idle0.png"))
        self.megabot["idle"].append(Texture("asset/megabot/idle1.png"))
        self.megabot["shoot"] = []
        self.megabot["shoot"].append(Texture("asset/megabot/shoot0.png"))
        self.megabot["jump"] = []
        self.megabot["jump"].append(Texture("asset/megabot/jump0.png"))
        self.megabot["jumpshoot"] = []
        self.megabot["jumpshoot"].append(Texture("asset/megabot/jumpshoot0.png"))
        self.bg = {}
        self.bg["ground"] = []
        self.bg["ground"].append(Texture("asset/bg/ground00.png"))
        self.bg["ground"].append(Texture("asset/bg/ground01.png"))
        self.bg["ground"].append(Texture("asset/bg/ground02.png"))
        self.bg["ground"].append(Texture("asset/bg/ground03.png"))
        self.bg["ground"].append(Texture("asset/bg/ground04.png"))
        self.bg["ground"].append(Texture("asset/bg/ground05.png"))
        self.bg["ground"].append(Texture("asset/bg/ground06.png"))
        self.bg["column"] = []
        self.bg["column"].append(Texture("asset/bg/column0.png"))
        self.bg["column"].append(Texture("asset/bg/column1.png"))
        self.bg["column"].append(Texture("asset/bg/column2.png"))
        self.bg["wall"] = []
        self.bg["wall"].append(Texture("asset/bg/wall0.png"))
        self.bg["wall"].append(Texture("asset/bg/wall1.png"))
        self.bg["wall"].append(Texture("asset/bg/wall2.png"))
        self.bg["color"] = []
        self.bg["color"].append(Texture("asset/bg/color00.png"))
        self.bg["color"].append(Texture("asset/bg/color01.png"))
        self.bg["color"].append(Texture("asset/bg/color02.png"))
        self.bg["color"].append(Texture("asset/bg/color03.png"))
        self.bg["color"].append(Texture("asset/bg/color04.png"))

        self.temp = {}

        # set up events
        self.on_key_down = self.event_keydown
        self.on_joy_button_down = self.event_joybuttondown

        # set up audio
        pygame.mixer.init()
#        pygame.mixer.music.load("asset/music/cinematic2.ogg")
#        pygame.mixer.music.play(-1)
        self.mode = None
        self.nextModeStart = None


    def update(self):
        global wiimote
        #TODO: sound

        if self.nextModeStart and Gloss.tick_count > self.nextModeStart:
            self.switchMode(self.nextMode, True)
        if self.mode == Megabot.MODE_QUIT:
            Gloss.game_is_running = False

        if self.mode == Megabot.MODE_TITLE:
            if wiimote:
                if wiimote.state["buttons"] & cwiid.BTN_2:
                    if not self.temp["jump"]:
                        self.temp["airstart"] = Gloss.tick_count
                    self.temp["jump"] = 1
                else:
                    self.temp["jump"] = 0
                if wiimote.state["buttons"] & cwiid.BTN_1:
                    self.temp["shootstart"] = Gloss.tick_count
                    self.temp["shoot"] = 1
#                else:
#                    self.temp["shoot"] = 0
            if self.temp["shootstart"] < Gloss.tick_count - 100:
                self.temp["shootstart"] = 0
                self.temp["shoot"] = 0
            if self.temp["airstart"] < Gloss.tick_count - 500:
                self.temp["airstart"] = 0
                self.temp["air"] = 0
            else:
                frac = (Gloss.tick_count - self.temp["airstart"]) / 500.0
                self.temp["air"] = Gloss.multi_lerp([0, 0.5, 0.9, 1, 0.9, 0.5, 0], frac)


    def event_joybuttondown(self, event):
        print(event)


    def event_keydown(self, event):
        if self.mode == Megabot.MODE_TITLE:
            self.switchMode(Megabot.MODE_QUIT)


    def draw(self):
        # clear screen
        Gloss.clear(Color.BLACK)

        # draw mode-specific
        if self.mode == Megabot.MODE_TITLE:
            self.drawTitle()
        elif self.mode == Megabot.MODE_GAME:
            self.drawGame()
        else:
            self.switchMode(Megabot.MODE_TITLE, True)

        # draw fade if we're switching mode
        if Gloss.tick_count < self.modeStart + Megabot.FADE_TIME:
            self.drawFade()
        elif (self.nextModeStart and
              Gloss.tick_count + Megabot.FADE_TIME > self.nextModeStart):
            self.drawFade(True)
        # draw letterbox masks
        self.drawMasks()


    def switchMode(self, mode, instant = False):
        if not instant:
            self.nextModeStart = Gloss.tick_count + Megabot.FADE_TIME
            self.nextMode = mode
            pygame.mixer.music.fadeout(Megabot.FADE_TIME)
            return

        self.modeStart = Gloss.tick_count
        self.mode = mode
        self.nextModeStart = None

        if mode == Megabot.MODE_TITLE:
            self.temp = {"blink": False, "jump": False,
                         "shoot": 0, "shootstart": 0,
                         "air": 0, "airstart": 0}
        elif mode == Megabot.MODE_GAME:
#            pygame.mixer.music.load("asset/music/folklore.ogg")
#            pygame.mixer.music.play(-1)
            pass
        else:
            self.temp.clear()


    def drawSpriteNES(self, nesPos, sprite, orig = (0, 0)):
        nesPos = Point.multiply(nesPos, self.scale)
        nesPos = Point.add(nesPos, self.display["pos"])
        sprite.draw(nesPos, origin = orig, scale = self.scale)


    def drawSpriteBlock(self, blockPos, sprite, orig = (0, 0)):
        return self.drawSpriteNES(Point.multiply(blockPos, 16), sprite, orig)


    def drawText(self, text, pos):
        pos = Point.add(pos, self.display["pos"])
        self.font.draw(text, Point.add(pos, (self.fontOffset, self.fontOffset)),
                       color = Color.BLACK)
        self.font.draw(text, pos, color = Color.WHITE)


    def drawTextNES(self, text, pos):
        return self.drawText(text, Point.multiply(pos, self.scale))


    def drawTextBlock(self, text, pos):
        return self.drawTextNES(text, Point.multiply(pos, 16))


    def drawMasks(self):
        for box in self.displayMask:
            Gloss.draw_box(box["pos"], color = Color.BLACK,
                           width = box["size"][0], height = box["size"][1])


    def drawFade(self, out = False):
        trans = Color(0, 0, 0, 0)
        if out:
            fade = (self.nextModeStart -
                    Gloss.tick_count) / float(Megabot.FADE_TIME)
        else:
            fade = (Gloss.tick_count -
                    self.modeStart) / float(Megabot.FADE_TIME)
        Gloss.draw_box(self.display["pos"], width = self.display["size"][0],
                       height = self.display["size"][1],
                       color = Color.smooth_step(Color.BLACK, trans, fade))


    def drawTitle(self):
        self.title.draw(self.display["pos"], scale = self.scale)

        for i in range(0, 20):
            self.drawSpriteBlock((i, 13), self.bg["ground"][3])
            self.drawSpriteBlock((i, 14), self.bg["ground"][0])
        for i in range(2, 7):
            for j in range(9, 13):
                if i == 2:
                    self.drawSpriteBlock((i, j), self.bg["wall"][0])
                elif i == 3 and j > 10:
                    self.drawSpriteBlock((i, j), self.bg["color"][0])
                elif i == 6:
                    self.drawSpriteBlock((i, j), self.bg["wall"][2])
                else:
                    self.drawSpriteBlock((i, j), self.bg["color"][4])
#        for i in range(2, 7):
#            self.drawSpriteBlock((i, 8), self.bg["ground"][1])

#        if self.temp["jump"]:
        x = 16 * 5
        y = 240 - (16 * 2)
        if self.temp["air"]:
            y = y - (self.temp["air"] * 48)
            sprite = self.megabot["jump"][0]
            if self.temp["shoot"]:
                sprite = self.megabot["jumpshoot"][0]
            self.drawSpriteNES((x, y), sprite, (12, 24))
        elif self.temp["shoot"]:
            self.drawSpriteNES((x, y), self.megabot["shoot"][0], (12, 24))
        elif self.temp["blink"]:
            self.drawSpriteNES((x, y), self.megabot["idle"][1], (12, 24))
        else:
            self.drawSpriteNES((x, y), self.megabot["idle"][0], (12, 24))

        text = "PRESS ANY KEY"
        self.drawTextBlock("PRESS ANY KEY", (5.5, 6))


    def drawGame(self):
        return


def parseOptions():
    parser = optparse.OptionParser()
    parser.add_option("-O", "--offsetx", dest = "offx", metavar = "OFFSET",
                      help = "adjust view offset (default is centered)")
    parser.add_option("-o", "--offsety", dest = "offy", metavar = "OFFSET",
                      help = "adjust view offset (default is centered)")
    parser.add_option("-w", "--window", dest = "window", action = "store_true",
                      help = "run in a window", default = False)
    parser.add_option("-r", "--resolution", dest = "res", metavar = "WxH",
                      help = "force screen/window resolution")
    parser.add_option("-f", "--filter", dest = "filter", action = "store_true",
                      help = "Filter (smooth/blur) scaling", default = False)
    return parser.parse_args()


def megabot():
    global options
    global wiimote

    game = Megabot("MEGABOT")

    # optionally rig up wiimote
    # TODO: event coalescing
    # TODO: nunchuk/classic
    # TODO: ps3 gamepad?
    try:
#        throw("foo")
        wiimote = cwiid.Wiimote()
        wiimote.led = 1
        wiimote.rumble = 1
        time.sleep(0.2)
        wiimote.rumble = 0
        wiimote.rpt_mode = cwiid.RPT_BTN #| RPT_EXT | RPT_CLASSIC
#        wiimote.close()    
    except:
        wiimote = None
        pass

    # set up screen
    if options.window:
        width = 768
        height = 720
        if options.res:
            try:
                (width, height) = options.res.lower().split("x")
            except:
                pass
        Gloss.screen_resolution = (int(width), int(height))
    else:
        Gloss.full_screen = True
        Gloss.screen_resolution = (0,0) #native/active resolution
    options.offset = (None, None)
    if options.offx:
        options.offset = (int(options.offx), options.offset[1])
    if options.offy:
        options.offset = (options.offset[0], int(options.offy))
    if not options.filter:
        Gloss.scaler = GL_NEAREST
    # here we go!
    game.run()


#main
(options, args) = parseOptions()
wiimote = None
megabot()
