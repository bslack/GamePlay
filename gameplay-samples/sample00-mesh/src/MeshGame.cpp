#include "MeshGame.h"

// Declare our game instance
MeshGame game;

MeshGame::MeshGame()
    : _font(NULL), _scene(NULL),_modelNode(NULL), _touched(false), _touchX(0)
{
}

MeshGame::~MeshGame()
{
}

void MeshGame::initialize()
{
    // Display the gameplay splash screen for at least 2.4 seconds.
    displaySplash(this, &MeshGame::drawSplash, NULL, 2400L);

    // Load font
    _font = Font::create("res/arial40.gpb");

    // Load mesh/scene from file
    Package* pkg = Package::create("res/duck.gpb");
    _scene = pkg->loadScene();
    SAFE_RELEASE(pkg);

    // Get the duck node
    _modelNode = _scene->findNode("duck");

    // Bind the material to the model
    _modelNode->getModel()->setMaterial("res/duck.material");

    // Find the light node
    Node* lightNode = _scene->findNode("directionalLight1");

    // Bind the light node's direction into duck's material.
    _modelNode->getModel()->getMaterial()->getParameter("u_lightDirection")->bindValue(lightNode, &Node::getForwardVectorView);
}

void MeshGame::finalize()
{
    SAFE_RELEASE(_font);
    SAFE_RELEASE(_scene);
}

void MeshGame::update(long elapsedTime)
{
    // Rotate model
    if (!_touched)
        _modelNode->rotateY(MATH_DEG_TO_RAD(0.5f));
}

void MeshGame::render(long elapsedTime)
{
    // Clear the color and depth buffers.
    clear(CLEAR_COLOR_DEPTH, Vector4::zero(), 1.0f, 0);

    // Visit all the nodes in the scene, drawing the models/mesh.
    _scene->visit(this, &MeshGame::drawScene);

    // Draw the fps
    drawFrameRate(_font, Vector4(0, 0.5f, 1, 1), 5, 5, getFrameRate());
}

void MeshGame::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
    switch (evt)
    {
    case Touch::TOUCH_PRESS:
        {
            _touched = true;
            _touchX = x;
        }
        break;
    case Touch::TOUCH_RELEASE:
        {
            _touched = false;
            _touchX = 0;
        }
        break;
    case Touch::TOUCH_MOVE:
        {
            int deltaX = x - _touchX;
            _touchX = x;
            _modelNode->rotateY(MATH_DEG_TO_RAD(deltaX * 0.5f));
        }
        break;
    default:
        break;
    };
}

bool MeshGame::drawScene(Node* node, void* cookie)
{
    Model* model = node->getModel();
    if (model)
        model->draw();
    return true;
}

void MeshGame::drawFrameRate(Font* font, const Vector4& color, unsigned int x, unsigned int y, unsigned int fps)
{
    char buffer[10];
    sprintf(buffer, "%u", fps);
    font->begin();
    font->drawText(buffer, x, y, color, font->getSize());
    font->end();
}

void MeshGame::drawSplash(void* param)
{
    clear(CLEAR_COLOR_DEPTH, Vector4(0, 0, 0, 1), 1.0f, 0);
    SpriteBatch* batch = SpriteBatch::create("res/logo_powered_white.png");
    batch->begin();
    batch->draw(this->getWidth() * 0.5f, this->getHeight() * 0.5f, 0.0f, 512.0f, 512.0f, 0.0f, 1.0f, 1.0f, 0.0f, Vector4::one(), true);
    batch->end();
    SAFE_DELETE(batch);
}
