#include "module/animation/frame/JOFrameUnpackPng.h"
#include "manager/JOFileMgr.h"
#include "utils/JOPath.h"
#include "utils/JOLog.h"

#include "cocos2d.h"
USING_NS_CC;

NS_JOFW_BEGIN

static std::unordered_map<std::string, Texture2D::PixelFormat> pixelFormats = {
		{ "RGBA8888", Texture2D::PixelFormat::RGBA8888 },
		{ "RGBA4444", Texture2D::PixelFormat::RGBA4444 },
		{ "RGB5A1", Texture2D::PixelFormat::RGB5A1 },
		{ "RGBA5551", Texture2D::PixelFormat::RGB5A1 },
		{ "RGB565", Texture2D::PixelFormat::RGB565 },
		{ "A8", Texture2D::PixelFormat::A8 },
		{ "ALPHA", Texture2D::PixelFormat::A8 },
		{ "I8", Texture2D::PixelFormat::I8 },
		{ "AI88", Texture2D::PixelFormat::AI88 },
		{ "ALPHA_INTENSITY", Texture2D::PixelFormat::AI88 },
		//{"BGRA8888", Texture2D::PixelFormat::BGRA8888}, no Image convertion RGBA -> BGRA
		{ "RGB888", Texture2D::PixelFormat::RGB888 }
};

JOFrameUnpackPng::JOFrameUnpackPng()
:m_isUnPacking(false)
{
	
}

JOFrameUnpackPng::~JOFrameUnpackPng()
{
	m_fileList.clear();
}

void JOFrameUnpackPng::unPack(const std::string& src, const std::string& dest)
{
	if (m_isUnPacking==true){
		LOG_WARN("JOFrameUnpackPng", "is unpacking !!!!");
		return;
	}
	
	m_destPath = dest;
	m_fileList.clear();
	JOFileMgr::getFilePathInDIR(src.c_str(), m_fileList, "plist");
	m_isUnPacking = true;
}

void JOFrameUnpackPng::tick()
{
	if (m_isUnPacking)
	{
		if (!m_fileList.empty())
		{
			std::string p = m_fileList.front();
			m_fileList.pop_front();

			std::string pixelFormatName;
			std::string texturePath("");

			ValueMap dict = FileUtils::getInstance()->getValueMapFromFile(p);
			ValueMap& framesDict = dict["frames"].asValueMap();
			Texture2D::PixelFormat pixelFormat = Texture2D::PixelFormat::RGBA8888;
			std::string destPath = JOPath::standardisePath(m_destPath, true);
			destPath += JOPath::getFileName(p, false) + "/";
			texturePath = "";

			if (dict.find("metadata") != dict.end())
			{
				ValueMap& metadataDict = dict.at("metadata").asValueMap();
				if (metadataDict.find("pixelFormat") != metadataDict.end())
				{
					pixelFormatName = metadataDict.at("pixelFormat").asString();
					if (pixelFormats.find(pixelFormatName) != pixelFormats.end())
					{
						pixelFormat = pixelFormats[pixelFormatName];
					}
				}
				texturePath = metadataDict["textureFileName"].asString();
			}

			if (!texturePath.empty())
			{
				// build texture path relative to plist file
				texturePath = FileUtils::getInstance()->fullPathFromRelativeFile(texturePath, p);
			}
			else
			{
				// build texture path by replacing file extension
				texturePath = p;

				// remove .xxx
				size_t startPos = texturePath.find_last_of(".");
				texturePath = texturePath.erase(startPos);

				// append .png
				texturePath = texturePath.append(".png");

				CCLOG("cocos2d: SpriteFrameCache: Trying to use file %s as texture", texturePath.c_str());
			}

			SpriteFrameCache::getInstance()->addSpriteFramesWithFile(p);
			std::string suffix = JO_EMTYP_STRING;
			Size aniSize = Size::ZERO;
			Point offset = Point::ZERO;
			for (auto iter = framesDict.begin(); iter != framesDict.end(); ++iter)
			{
				ValueMap& frameDict = iter->second.asValueMap();
				std::string spriteFrameName = iter->first;
				Sprite *fish = Sprite::createWithSpriteFrameName(spriteFrameName);
				//fish->setAnchorPoint(Vec2::ZERO);
				SpriteFrame* frame = fish->getSpriteFrame();
				aniSize = frame->getOriginalSizeInPixels(); // 原始图片大小
				offset = frame->getOffsetInPixels(); // 图片中心点的偏移量
				//CCRect rc = frame->getRectInPixels(); // 切图信息
	
				fish->setPosition(Point(aniSize.width*0.5f - offset.x, aniSize.height*0.5f - offset.y));
				RenderTexture *renderer = RenderTexture::create(aniSize.width, aniSize.height, pixelFormat);
				renderer->begin();
				fish->visit();
				renderer->end();
				JOFileMgr::MkMulDir(destPath);
				if (JOPath::getFileSuffix(spriteFrameName) == "jpg"){
					renderer->saveToFilePath(destPath + spriteFrameName, Image::Format::JPG, false);
				}
				else{
					renderer->saveToFilePath(destPath + spriteFrameName, Image::Format::PNG, true);
				}
			}
			SpriteFrameCache::getInstance()->removeSpriteFramesFromFile(p);
			Director::getInstance()->getTextureCache()->removeTextureForKey(texturePath);
			LOG_INFO("JOFrameUnpackPng", "plist [%s] unpack ok!", p.c_str());
		}
		else{
			m_isUnPacking = false;
			LOG_INFO("JOFrameUnpackPng", "all done !!!");
		}
	}
}

void JOFrameUnpackPng::cancelUnPack()
{
	m_isUnPacking = false;
	m_fileList.clear();
}

NS_JOFW_END