//
//  EditViewController.m
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/4/29.
//

#import "EditViewController.h"
#import <AVFoundation/AVFoundation.h>
#import "DisplayView.h"
#import "ThumbCell.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include "PreviewDecoder.hpp"
#include "TimeLineDecoder.hpp"
#include "ExportVideo.hpp"

using namespace std;

@interface EditViewController ()<UICollectionViewDelegate, UICollectionViewDataSource>
@property (nonatomic, strong) UICollectionView *thumbCollectionView;
@property (nonatomic, strong) DisplayView *displayView;
@property (nonatomic, weak) UIButton *playBtn;

@end

@implementation EditViewController
{
    int64_t currentVideoPts;
    int lastSeekRow;
    int decodedStartRow;
    int decodedEndRow;
    PreviewDecoder *previewDecoder;
    TimeLineDecoder *timeLineDecoder;
    ExportVideo *exportVideo;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [self setupView];
    
    __weak EditViewController *weakSelf = self;
    
    const char *inputFileName = [self.inputFileName cStringUsingEncoding:NSUTF8StringEncoding];
    previewDecoder = new PreviewDecoder(inputFileName, [weakSelf](AVFrame *display_frame) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [weakSelf.displayView displayFrame:display_frame]; // 播放
        });
    });
    
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0), ^{
        self->previewDecoder->videoPreviewDecode(0);
    });
    
    timeLineDecoder = new TimeLineDecoder(inputFileName);
    decodedStartRow = 0;
    decodedEndRow = 10;
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0), ^{
        self->timeLineDecoder->videoDecode(0, 10, [weakSelf](int startRow, int endRow) -> void {
            [weakSelf refreshCellsWithStart:startRow end:endRow];
        });
    });
    
    NSLog(@"NSHomeDirectory %@", NSHomeDirectory());
}

- (void)setupView {
    self.view.backgroundColor = UIColor.cyanColor;
    
    UIButton *closeBtn = [[UIButton alloc] initWithFrame:CGRectMake(20, 40, 60, 40)];
    closeBtn.layer.cornerRadius = 10;
    closeBtn.clipsToBounds = YES;
    closeBtn.backgroundColor = UIColor.purpleColor;
    [closeBtn setTitle:@"关闭" forState:UIControlStateNormal];
    [closeBtn addTarget:self action:@selector(closeBtnAction) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:closeBtn];
    
    UIButton *exportBtn = [[UIButton alloc] initWithFrame:CGRectMake(320, 40, 60, 40)];
    exportBtn.layer.cornerRadius = 10;
    exportBtn.clipsToBounds = YES;
    exportBtn.backgroundColor = UIColor.purpleColor;
    [exportBtn setTitle:@"导出" forState:UIControlStateNormal];
    [exportBtn addTarget:self action:@selector(exportBtnAction) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:exportBtn];
    
    CGFloat screenWidth = UIScreen.mainScreen.bounds.size.width;
    CGSize videosize = [self getVideoSize];
    CGFloat videoPixelWidth = videosize.width;
    CGFloat videoPixelHeight = videosize.height;
    CGFloat videoHeight = 240;
    CGFloat videoWidth = videoPixelWidth / videoPixelHeight * videoHeight;
    self.displayView = [[DisplayView alloc] initWithFrame:CGRectMake((screenWidth - videoWidth) * 0.5, 100, videoWidth, videoHeight) videoScale:videoPixelHeight / videoHeight];
    self.displayView.backgroundColor = UIColor.whiteColor;
    [self.view addSubview:self.displayView];
    
    UIButton *playBtn = [[UIButton alloc] initWithFrame:CGRectMake((screenWidth - 80) * 0.5, 350, 80, 40)];
    playBtn.layer.cornerRadius = 10;
    playBtn.clipsToBounds = YES;
    playBtn.backgroundColor = UIColor.blueColor;
    [playBtn setTitle:@"暂停" forState:UIControlStateNormal];
    [playBtn setTitle:@"播放" forState:UIControlStateSelected];
    [playBtn addTarget:self action:@selector(playBtnAction) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:playBtn];
    playBtn.selected = YES;
    self.playBtn = playBtn;
    
    int num = 8;
    CGFloat itemWidth = ceil(screenWidth / (CGFloat)num);
    CGFloat itemHeight = itemWidth;
    UICollectionViewFlowLayout *layout = [[UICollectionViewFlowLayout alloc] init];
    layout.itemSize = CGSizeMake(itemWidth, itemHeight);
    layout.minimumLineSpacing = 0;
    layout.minimumInteritemSpacing = 0;
    layout.scrollDirection = UICollectionViewScrollDirectionHorizontal;
    self.thumbCollectionView = [[UICollectionView alloc] initWithFrame:CGRectMake(0, 400, screenWidth, itemHeight) collectionViewLayout:layout];
    self.thumbCollectionView.delegate = self;
    self.thumbCollectionView.dataSource = self;
    self.thumbCollectionView.alwaysBounceHorizontal = YES;
    [self.thumbCollectionView registerClass:ThumbCell.class forCellWithReuseIdentifier:@"thumbCell"];
    [self.view addSubview:self.thumbCollectionView];
    
    UIButton *inversionBtn = [[UIButton alloc] initWithFrame:CGRectMake(20, 500, 100, 40)];
    inversionBtn.layer.cornerRadius = 10;
    inversionBtn.clipsToBounds = YES;
    inversionBtn.backgroundColor = UIColor.purpleColor;
    [inversionBtn setTitle:@"反相" forState:UIControlStateNormal];
    [inversionBtn addTarget:self action:@selector(inversionBtnAction) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:inversionBtn];
    
    UIButton *grayscaleBtn = [[UIButton alloc] initWithFrame:CGRectMake(140, 500, 100, 40)];
    grayscaleBtn.layer.cornerRadius = 10;
    grayscaleBtn.clipsToBounds = YES;
    grayscaleBtn.backgroundColor = UIColor.purpleColor;
    [grayscaleBtn setTitle:@"灰度" forState:UIControlStateNormal];
    [grayscaleBtn addTarget:self action:@selector(grayscaleBtnAction) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:grayscaleBtn];
    
    UIButton *noFilterBtn = [[UIButton alloc] initWithFrame:CGRectMake(260, 500, 100, 40)];
    noFilterBtn.layer.cornerRadius = 10;
    noFilterBtn.clipsToBounds = YES;
    noFilterBtn.backgroundColor = UIColor.purpleColor;
    [noFilterBtn setTitle:@"还原" forState:UIControlStateNormal];
    [noFilterBtn addTarget:self action:@selector(noFilterBtnAction) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:noFilterBtn];
    
    UIButton *stick1Btn = [[UIButton alloc] initWithFrame:CGRectMake(20, 580, 100, 40)];
    stick1Btn.layer.cornerRadius = 10;
    stick1Btn.clipsToBounds = YES;
    stick1Btn.backgroundColor = UIColor.purpleColor;
    [stick1Btn setTitle:@"老虎贴纸" forState:UIControlStateNormal];
    [stick1Btn addTarget:self action:@selector(stick1BtnAction) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:stick1Btn];
    
    UIButton *stick2Btn = [[UIButton alloc] initWithFrame:CGRectMake(140, 580, 100, 40)];
    stick2Btn.layer.cornerRadius = 10;
    stick2Btn.clipsToBounds = YES;
    stick2Btn.backgroundColor = UIColor.purpleColor;
    [stick2Btn setTitle:@"无人机贴纸" forState:UIControlStateNormal];
    [stick2Btn addTarget:self action:@selector(stick2BtnAction) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:stick2Btn];
    
    UIButton *effect1Btn = [[UIButton alloc] initWithFrame:CGRectMake(20, 660, 100, 40)];
    effect1Btn.layer.cornerRadius = 10;
    effect1Btn.clipsToBounds = YES;
    effect1Btn.backgroundColor = UIColor.purpleColor;
    [effect1Btn setTitle:@"二分特效" forState:UIControlStateNormal];
    [effect1Btn addTarget:self action:@selector(effect1BtnAction) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:effect1Btn];
    
    UIButton *effect2Btn = [[UIButton alloc] initWithFrame:CGRectMake(140, 660, 100, 40)];
    effect2Btn.layer.cornerRadius = 10;
    effect2Btn.clipsToBounds = YES;
    effect2Btn.backgroundColor = UIColor.purpleColor;
    [effect2Btn setTitle:@"四分特效" forState:UIControlStateNormal];
    [effect2Btn addTarget:self action:@selector(effect2BtnAction) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:effect2Btn];
    
    UIButton *noEffectBtn = [[UIButton alloc] initWithFrame:CGRectMake(260, 660, 100, 40)];
    noEffectBtn.layer.cornerRadius = 10;
    noEffectBtn.clipsToBounds = YES;
    noEffectBtn.backgroundColor = UIColor.purpleColor;
    [noEffectBtn setTitle:@"还原" forState:UIControlStateNormal];
    [noEffectBtn addTarget:self action:@selector(noEffectBtnAction) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:noEffectBtn];
}

- (void)closeBtnAction {
    [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)exportBtnAction {
    NSLog(@"begin export");
    if (!self.playBtn.selected) {
        self.playBtn.selected = YES;
        previewDecoder->setPause(true);
    }
    
    const char *inputFileName = [self.inputFileName cStringUsingEncoding:NSUTF8StringEncoding];
    NSString *documentPath = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    const char *outputPath = [[documentPath stringByAppendingPathComponent:@"output.mp4"] cStringUsingEncoding:NSUTF8StringEncoding];
    exportVideo = new ExportVideo(inputFileName, outputPath);
    exportVideo->videoRender = self.displayView->videoRender;
    exportVideo->videoDecode();
}

- (void)playBtnAction {
    
    self.playBtn.selected = !self.playBtn.isSelected;
    previewDecoder->setPause(self.playBtn.isSelected);

    if (self.playBtn.selected) {
        return;
    }
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^{
        self->previewDecoder->videoPlayDecode();
    });
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^{
        self->previewDecoder->audioPlayDecode();
    });
}

- (void)inversionBtnAction {
    self.displayView.applyInversionFilter = YES;
    self.displayView.applyGrayscaleFilter = NO;
}

- (void)grayscaleBtnAction {
    self.displayView.applyGrayscaleFilter = YES;
    self.displayView.applyInversionFilter = NO;
}

- (void)noFilterBtnAction {
    self.displayView.applyGrayscaleFilter = NO;
    self.displayView.applyInversionFilter = NO;
}

- (void)stick1BtnAction {
    self.displayView.applySticker1 = YES;
    self.displayView.applySticker2 = NO;
}

- (void)stick2BtnAction {
    self.displayView.applySticker2 = YES;
    self.displayView.applySticker1 = NO;
}

- (void)effect1BtnAction {
    self.displayView.applyEffect1 = YES;
    self.displayView.applyEffect2 = NO;
}

- (void)effect2BtnAction {
    self.displayView.applyEffect1 = NO;
    self.displayView.applyEffect2 = YES;
}

- (void)noEffectBtnAction {
    self.displayView.applyEffect1 = NO;
    self.displayView.applyEffect2 = NO;
}

#pragma mark - delegate
- (NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section {
    return 242;
}

- (__kindof UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath {
    ThumbCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier:@"thumbCell" forIndexPath:indexPath];
    return cell;
}

- (void)collectionView:(UICollectionView *)collectionView willDisplayCell:(UICollectionViewCell *)cell forItemAtIndexPath:(NSIndexPath *)indexPath {
    AVFrame *display_frame = timeLineDecoder->frameForIndex((int)indexPath.row);
    if (display_frame) {
        ((ThumbCell *)cell).thumbImgView.image = [self imageFromAVFrame:display_frame];
    }
}

- (void)scrollViewDidScroll:(UIScrollView *)scrollView {
    
    CGFloat offsetX = self.thumbCollectionView.contentOffset.x;
    CGFloat screenWidth = UIScreen.mainScreen.bounds.size.width;
    int num = 8;
    CGFloat itemWidth = screenWidth / (CGFloat)num;
    int seekRow = (int)offsetX / itemWidth;
    previewDecoder->seekRow = seekRow;
    
    if (!self.playBtn.selected) {
        self.playBtn.selected = YES;
        previewDecoder->setPause(true);
    }
    
    if (seekRow != lastSeekRow) {
        dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0), ^{
            self->previewDecoder->videoPreviewDecode(seekRow);
        });
    }
    lastSeekRow = seekRow;
    
    __weak EditViewController *weakSelf = self;
    
    if (decodedStartRow + 60 > seekRow && decodedStartRow > 0) {
        int start = MAX(self->decodedStartRow - 30, 0);
        int end = decodedStartRow - 1;
        if (start > end) {
            return;
        }

        dispatch_async(dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^{
            self->timeLineDecoder->videoDecode(start, end, [weakSelf](int startRow, int endRow) -> void {
                [weakSelf refreshCellsWithStart:startRow end:endRow];
            });
        });

        decodedStartRow = MAX(self->decodedStartRow - 30, 0);
    }

    if (seekRow + 60 > decodedEndRow && decodedEndRow < 241) {
        int start = self->decodedEndRow + 1;
        int end = MIN(self->decodedEndRow + 30, 241);
        if (start > end) {
            return;
        }

        dispatch_async(dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^{
            self->timeLineDecoder->videoDecode(start, end, [weakSelf](int startRow, int endRow) -> void {
                [weakSelf refreshCellsWithStart:startRow end:endRow];
            });
        });

        decodedEndRow = MIN(self->decodedEndRow + 30, 241);
    }
}

- (UIImage *)imageFromAVFrame:(AVFrame *)frame {
    if (!frame) return nil;
    
    int frameWidth = frame->width;
    int frameHeight = frame->height;
    
    if (frameWidth == 0 || frameHeight == 0) {
        return nil;
    }
    
    // 创建RGB数据缓冲区
    uint8_t *rgbaBuffer = (uint8_t*)malloc(frameWidth * frameHeight * 4); // 3 bytes per pixel for RGB
    if (!rgbaBuffer) {
        // 内存分配失败，处理错误
    }
    
    // 设置转换参数
    struct SwsContext *sws_ctx = sws_getContext(frameWidth, frameHeight, AV_PIX_FMT_YUV420P,
                                                frameWidth, frameHeight, AV_PIX_FMT_RGBA,
                                                SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws_ctx) {
        // 创建转换上下文失败，处理错误
    }
    
    int dstStride[AV_NUM_DATA_POINTERS] = {0};
    dstStride[0] = frameWidth * 4;
    // 执行转换
    sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height,
              &rgbaBuffer, dstStride);
    
    // 创建位图上下文
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(rgbaBuffer, frameWidth, frameHeight, 8, dstStride[0], colorSpace, kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast);
    
    // 创建图像
    CGImageRef cgImage = CGBitmapContextCreateImage(context);
    UIImage *image = [UIImage imageWithCGImage:cgImage];
    
    // 释放资源
    CGImageRelease(cgImage);
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
    free(rgbaBuffer);
    
    return image;
}

- (CGSize)getVideoSize {
    CGSize size = CGSizeZero;
    
    AVFormatContext *videoifmt = NULL;
    const char *inputFileName = [self.inputFileName cStringUsingEncoding:NSUTF8StringEncoding];
    int ret = avformat_open_input(&videoifmt, inputFileName, NULL, NULL);
    if (ret < 0) {
        NSLog(@"open inputfile failed");
        return size;
    }
    
    int videoIndex = av_find_best_stream(videoifmt, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoIndex < 0) {
        NSLog(@"not find input video stream");
        return size;
    }
    
    AVStream *videoStram = videoifmt->streams[videoIndex];
    AVCodecParameters *videoCodecpar = videoStram->codecpar;
    size = CGSizeMake(videoCodecpar->width, videoCodecpar->height);
    avformat_close_input(&videoifmt);
    
    return size;
}

- (void)refreshCellsWithStart:(int)startRow end:(int)endRow {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSMutableArray *arr = [NSMutableArray array];
        for(int k = startRow; k <= endRow; k++) {
            [arr addObject:[NSIndexPath indexPathForRow:k inSection:0]];
        }
        [self.thumbCollectionView reloadItemsAtIndexPaths:arr];
    });
}

@end
