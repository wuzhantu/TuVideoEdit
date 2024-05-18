//
//  ViewController.m
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/4/29.
//

#import "ViewController.h"
#import "EditViewController.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [self setupView];
}

- (void)setupView {
    UIButton *startBtn = [[UIButton alloc] initWithFrame:CGRectMake(150, 200, 100, 50)];
    startBtn.backgroundColor = UIColor.purpleColor;
    [startBtn setTitle:@"开始创作" forState:UIControlStateNormal];
    [startBtn addTarget:self action:@selector(startBtnAction) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:startBtn];
}

- (void)startBtnAction {
    EditViewController *editVC = [EditViewController new];
    editVC.inputFileName = [[NSBundle mainBundle] pathForResource:@"xiaolin.MP4" ofType:nil]; // xiaolin.MP4(1172x720) snow.MP4(720*1280) xinwen.MP4(720*1280)
    editVC.modalPresentationStyle = UIModalPresentationFullScreen;
    [self presentViewController:editVC animated:YES completion:nil];
}

@end
