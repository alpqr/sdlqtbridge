#include "sdlqtbridge.h"
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFramebufferObject>

class State
{
public:
    State() : app(0) { }

    QGuiApplication *app;
};

Q_GLOBAL_STATIC(State, sdlqt)

class Renderer
{
public:
    Renderer()
        : m_context(0),
          m_nativeContext(0),
          m_pbuffer(0),
          m_fbo(0)
        { }

    ~Renderer() {
        delete m_fbo;
        delete m_pbuffer;
        delete m_context;
    }

    bool init(const QSurfaceFormat &format) {
        m_context = new QOpenGLContext;
        m_context->setFormat(format);
        bool ok = m_context->create();
        if (ok) {
            QPlatformNativeInterface *iface = qGuiApp->platformNativeInterface();
#if defined(Q_OS_WIN)
            QByteArray key = QByteArrayLiteral("renderingContext");
#elif defined(Q_OS_LINUX)
            QByteArray key = QByteArrayLiteral("glxcontext");
#endif
            void *ctx = iface->nativeResourceForContext(key, m_context);
            m_nativeContext = reinterpret_cast<SDL_GLContext>(ctx);

            m_pbuffer = new QOffscreenSurface;
            m_pbuffer->setFormat(format);
            m_pbuffer->create();
        }
        return ok;
    }

    SDL_GLContext context() const {
        return m_nativeContext;
    }

    bool makeCurrent() {
        return m_context->makeCurrent(m_pbuffer);
    }

    GLuint ensureFramebuffer(const QSize &size) {
        if (m_fbo && m_fbo->size() != size) {
            delete m_fbo;
            m_fbo = 0;
        }
        if (!m_fbo) {
            m_fbo = new QOpenGLFramebufferObject(size, QOpenGLFramebufferObject::CombinedDepthStencil);
            if (!m_fbo->isValid()) {
                delete m_fbo;
                m_fbo = 0;
                return 0;
            }
        }
        return m_fbo->handle();
    }

    void bindFramebuffer() {
        m_fbo->bind();
    }

    void releaseFramebuffer() {
        m_fbo->release();
    }

    GLuint texture() const {
        return m_fbo ? m_fbo->texture() : 0;
    }

private:
    QOpenGLContext *m_context;
    SDL_GLContext m_nativeContext;
    QOffscreenSurface *m_pbuffer;
    QOpenGLFramebufferObject *m_fbo;
};

extern "C" {

int sdlqt_init(int argc, char **argv)
{
    State *d = sdlqt();
    d->app = new QGuiApplication(argc, argv);
    return 0;
}

void sdlqt_quit(void)
{
    State *d = sdlqt();
    delete d->app;
}

sdlqt_renderer sdlqt_create_renderer(void)
{
    QSurfaceFormat fmt;
    int val;
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    if (!SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &val))
        fmt.setRedBufferSize(val);
    if (!SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &val))
        fmt.setGreenBufferSize(val);
    if (!SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &val))
        fmt.setBlueBufferSize(val);
    if (!SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &val))
        fmt.setAlphaBufferSize(val);
    if (!SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &val))
        fmt.setDepthBufferSize(val);
    if (!SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &val))
        fmt.setStencilBufferSize(val);
    if (!SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &val))
        fmt.setAlphaBufferSize(val);
    if (!SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &val)) {
        if (val && !SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &val))
            fmt.setSamples(val);
    }
    if (!SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &val))
        fmt.setMajorVersion(val);
    if (!SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &val))
        fmt.setMinorVersion(val);
    if (!SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &val)) {
        if (val & SDL_GL_CONTEXT_DEBUG_FLAG)
            fmt.setOption(QSurfaceFormat::DebugContext);
        if (fmt.majorVersion() >= 3 && !(val & SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG))
            fmt.setOption(QSurfaceFormat::DeprecatedFunctions);
    }
    if (!SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &val)) {
        switch (val) {
        case SDL_GL_CONTEXT_PROFILE_CORE:
            fmt.setProfile(QSurfaceFormat::CoreProfile);
            break;
        case SDL_GL_CONTEXT_PROFILE_COMPATIBILITY:
            fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
            break;
        case SDL_GL_CONTEXT_PROFILE_ES:
            fmt.setRenderableType(QSurfaceFormat::OpenGLES);
            break;
        default:
            break;
        }
    }

    Renderer *r = new Renderer;
    if (!r->init(fmt)) {
        delete r;
        r = 0;
    }
    return r;
}

void sdlqt_delete_renderer(sdlqt_renderer renderer)
{
    if (renderer)
        delete static_cast<Renderer *>(renderer);
}

SDL_GLContext sdlqt_get_context(sdlqt_renderer renderer)
{
    return renderer ? static_cast<Renderer *>(renderer)->context() : 0;
}

unsigned int sdlqt_get_texture(sdlqt_renderer renderer)
{
    Renderer *r = static_cast<Renderer *>(renderer);
    return r ? r->texture() : 0;
}

void sdlqt_render_offscreen(sdlqt_renderer renderer, int width, int height, sdlqt_render_func func)
{
    Renderer *r = static_cast<Renderer *>(renderer);
    if (!r || !r->makeCurrent())
        return;
    GLuint fbo = r->ensureFramebuffer(QSize(width, height));
    if (fbo) {
        r->bindFramebuffer();
        func(fbo);
        r->releaseFramebuffer();
    }
    SDL_GL_MakeCurrent(0, 0);
}

} // extern "C"
