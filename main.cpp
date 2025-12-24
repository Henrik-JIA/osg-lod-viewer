/**
 * OSG LOD (Level of Detail) 模型加载示例
 * 
 * 本示例展示如何使用OpenSceneGraph加载和显示LOD模型
 * 支持加载OSGB格式的倾斜摄影数据
 */

#include <osg/LOD>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osg/PolygonMode>
#include <osg/Light>
#include <osg/LightSource>
#include <osgDB/ReadFile>
#include <osgDB/Registry>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osgGA/GUIEventHandler>
#include <iostream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

/**
 * 设置明亮的场景光照
 */
void setupBrightLighting(osgViewer::Viewer& viewer, osg::Node* scene)
{
    // 设置浅色背景
    // viewer.getCamera()->setClearColor(osg::Vec4(0.5f, 0.6f, 0.7f, 1.0f));
    
    // 创建明亮的主光源
    osg::ref_ptr<osg::Light> light = new osg::Light();
    light->setLightNum(0);
    light->setPosition(osg::Vec4(1.0f, 1.0f, 1.0f, 0.0f));  // 方向光
    light->setAmbient(osg::Vec4(0.6f, 0.6f, 0.6f, 1.0f));   // 强环境光
    light->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));   // 强漫射光
    light->setSpecular(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
    
    osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource();
    lightSource->setLight(light.get());
    
    // 将光源添加到场景
    osg::Group* group = scene->asGroup();
    if (group)
    {
        group->addChild(lightSource.get());
    }
    
    // 启用光照和颜色材质
    osg::StateSet* ss = scene->getOrCreateStateSet();
    ss->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    ss->setMode(GL_LIGHT0, osg::StateAttribute::ON);
    ss->setMode(GL_COLOR_MATERIAL, osg::StateAttribute::ON);
}

/**
 * 显示模式切换（填充/线框/点）
 */
class DisplayModeHandler : public osgGA::GUIEventHandler
{
public:
    DisplayModeHandler(osg::Node* scene) : _scene(scene), _mode(0)
    {
        // 为每种模式创建单独的PolygonMode对象
        _fillMode = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
        _lineMode = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
        _pointMode = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::POINT);
    }

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&)
    {
        if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
        {
            if (ea.getKey() == 'f' || ea.getKey() == 'F')
            {
                _mode = (_mode + 1) % 3;
                osg::StateSet* ss = _scene->getOrCreateStateSet();
                
                if (_mode == 0) {
                    // 填充模式
                    ss->setAttributeAndModes(_fillMode.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                    std::cout << ">>> 填充模式" << std::endl;
                } else if (_mode == 1) {
                    // 线框模式
                    ss->setAttributeAndModes(_lineMode.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                    std::cout << ">>> 线框模式" << std::endl;
                } else {
                    // 点模式
                    ss->setAttributeAndModes(_pointMode.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                    std::cout << ">>> 点模式" << std::endl;
                }
                return true;
            }
        }
        return false;
    }

private:
    osg::Node* _scene;
    int _mode;
    osg::ref_ptr<osg::PolygonMode> _fillMode;
    osg::ref_ptr<osg::PolygonMode> _lineMode;
    osg::ref_ptr<osg::PolygonMode> _pointMode;
};

/**
 * 创建一个简单的几何体作为LOD的一个级别
 */
osg::ref_ptr<osg::Geode> createShapeGeode(osg::Shape* shape, const osg::Vec4& color)
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    
    osg::ref_ptr<osg::ShapeDrawable> drawable = new osg::ShapeDrawable(shape);
    drawable->setColor(color);
    geode->addDrawable(drawable.get());
    
    osg::ref_ptr<osg::Material> material = new osg::Material();
    material->setDiffuse(osg::Material::FRONT_AND_BACK, color);
    material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    material->setShininess(osg::Material::FRONT_AND_BACK, 64.0f);
    geode->getOrCreateStateSet()->setAttributeAndModes(material.get());
    
    return geode;
}

/**
 * 创建一个带有多个细节级别的LOD节点（演示用）
 */
osg::ref_ptr<osg::LOD> createLODNode()
{
    osg::ref_ptr<osg::LOD> lodNode = new osg::LOD();
    
    osg::ref_ptr<osg::Sphere> sphere = new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), 2.0f);
    osg::ref_ptr<osg::Geode> highDetail = createShapeGeode(sphere.get(), 
        osg::Vec4(0.2f, 0.8f, 0.2f, 1.0f));
    
    osg::ref_ptr<osg::Cylinder> cylinder = new osg::Cylinder(osg::Vec3(0.0f, 0.0f, 0.0f), 1.5f, 3.0f);
    osg::ref_ptr<osg::Geode> mediumDetail = createShapeGeode(cylinder.get(), 
        osg::Vec4(0.8f, 0.8f, 0.2f, 1.0f));
    
    osg::ref_ptr<osg::Box> box = new osg::Box(osg::Vec3(0.0f, 0.0f, 0.0f), 2.5f);
    osg::ref_ptr<osg::Geode> lowDetail = createShapeGeode(box.get(), 
        osg::Vec4(0.8f, 0.2f, 0.2f, 1.0f));
    
    lodNode->addChild(highDetail.get(), 0.0f, 50.0f);
    lodNode->addChild(mediumDetail.get(), 50.0f, 150.0f);
    lodNode->addChild(lowDetail.get(), 150.0f, 500.0f);
    
    return lodNode;
}

/**
 * 创建演示场景
 */
osg::ref_ptr<osg::Group> createDemoScene()
{
    osg::ref_ptr<osg::Group> root = new osg::Group();
    
    const float spacing = 15.0f;
    const int gridSize = 3;
    
    for (int i = 0; i < gridSize; ++i)
    {
        for (int j = 0; j < gridSize; ++j)
        {
            osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform();
            float x = (i - gridSize / 2) * spacing;
            float y = (j - gridSize / 2) * spacing;
            transform->setMatrix(osg::Matrix::translate(x, y, 0.0f));
            transform->addChild(createLODNode().get());
            root->addChild(transform.get());
        }
    }
    
    return root;
}

/**
 * 查找OSGB数据集的根文件
 * 支持查找 Model.osgb 或目录中的 Tile_*.osgb 文件
 */
std::string findOsgbRootFile(const std::string& path)
{
    fs::path inputPath(path);
    
    // 如果直接是文件，返回它
    if (fs::is_regular_file(inputPath))
    {
        return path;
    }
    
    // 如果是目录，查找根文件
    if (fs::is_directory(inputPath))
    {
        // 首先查找 Model.osgb
        fs::path modelFile = inputPath / "Model.osgb";
        if (fs::exists(modelFile))
        {
            return modelFile.string();
        }
        
        // 查找 Data 子目录下的 Model.osgb
        fs::path dataModelFile = inputPath / "Data" / "Model.osgb";
        if (fs::exists(dataModelFile))
        {
            return dataModelFile.string();
        }
        
        // 查找任何 .osgb 文件
        for (const auto& entry : fs::directory_iterator(inputPath))
        {
            if (entry.path().extension() == ".osgb")
            {
                return entry.path().string();
            }
        }
        
        // 在 Data 子目录中查找
        fs::path dataDir = inputPath / "Data";
        if (fs::is_directory(dataDir))
        {
            for (const auto& entry : fs::directory_iterator(dataDir))
            {
                if (entry.is_directory())
                {
                    // 查找 Tile_xxx/Tile_xxx.osgb 格式的根文件
                    std::string tileName = entry.path().filename().string();
                    fs::path tileRoot = entry.path() / (tileName + ".osgb");
                    if (fs::exists(tileRoot))
                    {
                        // 返回父目录，让OSG加载整个数据集
                        return (inputPath / "Model.osgb").string();
                    }
                }
            }
        }
    }
    
    return path;
}

/**
 * 加载OSGB模型文件
 */
osg::ref_ptr<osg::Node> loadOsgbModel(const std::string& path)
{
    std::string filePath = findOsgbRootFile(path);
    
    std::cout << "尝试加载: " << filePath << std::endl;
    
    // 设置数据库分页器选项，用于大型LOD数据集
    osg::ref_ptr<osgDB::Options> options = new osgDB::Options();
    options->setObjectCacheHint(osgDB::Options::CACHE_ALL);
    
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(filePath, options.get());
    
    if (model.valid())
    {
        std::cout << "成功加载模型!" << std::endl;
        
        // 打印模型边界信息
        osg::BoundingSphere bs = model->getBound();
        std::cout << "模型边界球:" << std::endl;
        std::cout << "  中心: (" << bs.center().x() << ", " 
                  << bs.center().y() << ", " << bs.center().z() << ")" << std::endl;
        std::cout << "  半径: " << bs.radius() << std::endl;
    }
    else
    {
        std::cerr << "加载模型失败: " << filePath << std::endl;
    }
    
    return model;
}

/**
 * 打印使用帮助
 */
void printUsage(const char* programName)
{
    std::cout << "=== OSG LOD 模型查看器 ===" << std::endl;
    std::cout << std::endl;
    std::cout << "用法: " << programName << " [模型路径]" << std::endl;
    std::cout << std::endl;
    std::cout << "支持的格式:" << std::endl;
    std::cout << "  - OSGB 文件 (*.osgb)" << std::endl;
    std::cout << "  - OSG 文件 (*.osg)" << std::endl;
    std::cout << "  - 倾斜摄影数据目录 (包含 Model.osgb)" << std::endl;
    std::cout << std::endl;
    std::cout << "示例:" << std::endl;
    std::cout << "  " << programName << " Data/model-osgb" << std::endl;
    std::cout << "  " << programName << " Data/model-osgb/Model.osgb" << std::endl;
    std::cout << "  " << programName << " Data/jinghe-model-osgb-refine" << std::endl;
    std::cout << std::endl;
}

// ============================================
// 在这里修改默认的模型路径（使用绝对路径更可靠）
// ============================================
const std::string DEFAULT_MODEL_PATH = "D:/Github_code/OSG_Test/Data/model-osgb/Model.osgb";

/**
 * 获取可执行文件所在目录
 */
std::string getExecutableDir(const char* argv0)
{
    fs::path exePath(argv0);
    if (exePath.has_parent_path())
    {
        return exePath.parent_path().string();
    }
    return ".";
}

/**
 * 尝试多个可能的路径来查找模型文件
 */
std::string findModelPath(const std::string& relativePath, const std::string& exeDir)
{
    // 尝试的路径列表
    std::vector<std::string> searchPaths = {
        relativePath,                                          // 直接使用相对路径
        exeDir + "/" + relativePath,                          // 相对于exe目录
        exeDir + "/../" + relativePath,                       // exe上级目录（build目录情况）
        exeDir + "/../../" + relativePath,                    // exe上两级目录
        "D:/Github_code/OSG_Test/" + relativePath,            // 绝对路径
    };
    
    for (const auto& path : searchPaths)
    {
        fs::path p(path);
        if (fs::exists(p))
        {
            std::cout << "找到模型文件: " << fs::absolute(p).string() << std::endl;
            return fs::absolute(p).string();
        }
    }
    
    // 如果都没找到，返回原始路径
    return relativePath;
}

int main(int argc, char** argv)
{
    printUsage(argv[0]);
    
    std::cout << "操作说明:" << std::endl;
    std::cout << "  - 鼠标左键拖动: 旋转视角" << std::endl;
    std::cout << "  - 鼠标中键拖动: 平移视角" << std::endl;
    std::cout << "  - 鼠标右键拖动/滚轮: 缩放视角" << std::endl;
    std::cout << "  - 按 F 键: 切换显示模式 (填充/线框/点)" << std::endl;
    std::cout << std::endl;
    
    // 创建Viewer
    osgViewer::Viewer viewer;
    
    osg::ref_ptr<osg::Node> scene;
    
    // 获取exe所在目录
    std::string exeDir = getExecutableDir(argv[0]);
    std::cout << "可执行文件目录: " << exeDir << std::endl;
    
    // 确定要加载的模型路径
    std::string modelPath;
    if (argc > 1)
    {
        // 优先使用命令行参数
        modelPath = argv[1];
        // 如果是相对路径，尝试查找
        if (!fs::path(modelPath).is_absolute())
        {
            modelPath = findModelPath(modelPath, exeDir);
        }
    }
    else
    {
        // 使用默认路径
        std::cout << "使用默认模型路径: " << DEFAULT_MODEL_PATH << std::endl;
        
        // 如果默认路径是绝对路径且存在，直接使用
        if (fs::path(DEFAULT_MODEL_PATH).is_absolute() && fs::exists(DEFAULT_MODEL_PATH))
        {
            modelPath = DEFAULT_MODEL_PATH;
        }
        else
        {
            // 否则尝试查找
            modelPath = findModelPath(DEFAULT_MODEL_PATH, exeDir);
        }
    }
    
    // 加载模型
    scene = loadOsgbModel(modelPath);
    
    // 如果没有加载成功，使用演示场景
    if (!scene.valid())
    {
        std::cout << "无法加载模型，使用演示场景..." << std::endl;
        std::cout << "  * 绿色球体 = 高细节 (距离 0-50)" << std::endl;
        std::cout << "  * 黄色圆柱 = 中细节 (距离 50-150)" << std::endl;
        std::cout << "  * 红色立方体 = 低细节 (距离 150+)" << std::endl;
        scene = createDemoScene();
    }
    
    // 设置场景数据
    viewer.setSceneData(scene.get());
    
    // 设置明亮的光照
    setupBrightLighting(viewer, scene.get());
    
    // 使用轨迹球操作器
    viewer.setCameraManipulator(new osgGA::TrackballManipulator());
    
    // 添加显示模式切换（F键：填充/线框/点）
    viewer.addEventHandler(new DisplayModeHandler(scene.get()));
    
    // 设置窗口
    viewer.setUpViewInWindow(100, 100, 1280, 720);
    
    // 自动调整相机到合适位置
    viewer.home();
    
    std::cout << std::endl << "开始渲染..." << std::endl;
    
    // 运行viewer
    return viewer.run();
}
