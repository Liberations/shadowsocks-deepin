//
// Created by pikachu on 17-7-20.
//

#include "SystemTrayIcon.h"
#include "EditServerWidget.h"
#include "EditServerDialog.h"
void output(Profile& profile){
    qDebug()<<"profile";
    qDebug()<<"server"<<profile.server<<profile.server_port<<profile.password;
    qDebug()<<"local "<<profile.local_address<<profile.local_port;
    qDebug()<<"method"<<profile.method;
    qDebug()<<"timeout"<<profile.timeout;
}
SystemTrayIcon::SystemTrayIcon(QObject *parent)
        : QSystemTrayIcon(parent),
          networkInter("com.deepin.daemon.Network", "/com/deepin/daemon/Network", QDBusConnection::sessionBus(), this) {
    setIcon(QIcon(":/icons/shadowsocks.ico"));
    menu = new QMenu("menu");
    startSystemAgentAction = new QAction("启动系统代理", this);
    startSystemAgentAction->setCheckable(true);
    menu->addAction(startSystemAgentAction);

    systemAgentModeMenu = new QMenu("系统代理模式", menu);
    QActionGroup* systemAgentModeActionGroup=new QActionGroup(this);
    pacModeAction = new QAction("PAC模式", this);
    globelModeAction = new QAction("全局模式", this);
    systemAgentModeActionGroup->addAction(pacModeAction);
    systemAgentModeActionGroup->addAction(globelModeAction);
    pacModeAction->setCheckable(true);
    globelModeAction->setCheckable(true);
    systemAgentModeMenu->addAction(pacModeAction);
    systemAgentModeMenu->addAction(globelModeAction);
    menu->addMenu(systemAgentModeMenu);

    serverMenu = new QMenu("服务器", menu);
    loadBalancingAction = new QAction("负载均衡", this);
    highAvailabilityAction = new QAction("高可用", this);
    accordingToStatisticsAction = new QAction("根据统计", this);
    serverGroup=new QActionGroup(this);
    editServerAction = new QAction("编辑服务器...", this);
    statisticsConfigurationAction = new QAction("统计配置...", this);
    shareServerConfigurationAction = new QAction("分享服务器配置...", this);
    scanThe2DCodeOnTheScreenAction = new QAction("扫描屏幕上的二维码...", this);
    importTheURLFromTheClipboardAction = new QAction("从剪贴板导入URL...", this);
    menu->addMenu(serverMenu);

    pacMenu = new QMenu("PAC", menu);
    useLocalPacAction = new QAction("使用本地PAC", this);
    useOnlinePacAction = new QAction("使用在线PAC", this);
    editLocalPacFileAction = new QAction("编辑本地PAC文件...", this);
    updateLocalPacFromGFWListAction = new QAction("从GFWList更新本地PAC", this);
    editUserRulesForGFWListAction = new QAction("编辑GFWList的用户规则...", this);
    protectLocalPacAction = new QAction("保护本地PAC", this);
    copyLocalPacUrlAction = new QAction("复制本地PAC网址", this);
    editOnlinePacUrlAction = new QAction("编辑在线PAC网址...", this);
    pacMenu->addAction(useLocalPacAction);
    pacMenu->addAction(useOnlinePacAction);
    pacMenu->addSeparator();
    pacMenu->addAction(editLocalPacFileAction);
    pacMenu->addAction(updateLocalPacFromGFWListAction);
    pacMenu->addAction(editUserRulesForGFWListAction);
    pacMenu->addAction(protectLocalPacAction);
    pacMenu->addAction(copyLocalPacUrlAction);
    pacMenu->addAction(editOnlinePacUrlAction);
    menu->addMenu(pacMenu);

    positiveAgentAction = new QAction("正向代理...", this);
    menu->addAction(positiveAgentAction);

    menu->addSeparator();

    bootAction = new QAction("开机启动", this);
    menu->addAction(bootAction);

    allowConnectionsFromTheLANAction = new QAction("允许来自局域网的连接", this);
    menu->addAction(allowConnectionsFromTheLANAction);

    menu->addSeparator();

    editShortcutsAction = new QAction("编辑快捷键...", this);
    menu->addAction(editShortcutsAction);

    helpMenu = new QMenu("帮助", menu);
    showLogAction = new QAction("显示日志", this);
    detailedLogAction = new QAction("详细记录日志", this);
    updateMenu = new QMenu("更新", helpMenu);
    checkForUpdateAction = new QAction("检查更新", this);
    updateMenu->addAction(checkForUpdateAction);
    updateMenu->addSeparator();
    checkForUpdatesAtStartupAction = new QAction("启动时检查更新", this);
    checkTheBetaUpdateAction = new QAction("检查测试版更新", this);
    updateMenu->addAction(checkForUpdatesAtStartupAction);
    updateMenu->addAction(checkTheBetaUpdateAction);
    helpMenu->addAction(showLogAction);
    helpMenu->addAction(detailedLogAction);
    helpMenu->addMenu(updateMenu);
    menu->addMenu(helpMenu);

    aboutAction = new QAction("关于", this);
    menu->addAction(aboutAction);

    exitAction = new QAction("退出", this);
    menu->addAction(exitAction);

    setContextMenu(menu);


    startSystemAgentAction->setChecked(false);
    globelModeAction->setChecked(true);
    systemAgentModeMenu->setEnabled(false);

    controller=new Controller(true);
        QObject::connect(controller, &QSS::Controller::debug, [](QString log) {
        qDebug() << "[QSS::Controller::debug]" << log;
    });
    QObject::connect(controller, &QSS::Controller::info, [](QString log) {
        qDebug() << "[QSS::Controller::info]" << log;
    });
//    editServerDialog=new EditServerDialog(profiles);
    initConfig();

    connect(startSystemAgentAction, &QAction::triggered, [this](bool checked) {

        if (!checked) {
            qDebug() << "取消系统代理";
            systemAgentModeMenu->setEnabled(false);
            setProxyMethod("none");
            controller->stop();
        } else {
            qDebug() << "启动系统代理";
            systemAgentModeMenu->setEnabled(true);
            if (pacModeAction->isChecked()){
                setAutoProxy();
            } else{
                setManualProxy();
            }

            controller->start();
        }

    });
    connect(systemAgentModeActionGroup,&QActionGroup::triggered,[this](QAction *action){
        if(action==pacModeAction){
            setAutoProxy();
        } else{
            setManualProxy();
        }
    });
    connect(serverGroup,&QActionGroup::triggered,this,&SystemTrayIcon::onServerActionTriggered);
    connect(editServerAction,&QAction::triggered,[this](){
//        EditServerDialog* w = new EditServerDialog(profiles, nullptr);
//        w->show();
        EditServerDialog* w = new EditServerDialog();
        int ret=w->exec();
        if(ret==QDialog::Accepted){
#ifdef QT_DEBUG
            qDebug()<<"保存配置";
#endif
            configs=ConfigUtil::readConfig();
            updateServerMenu();
            if(startSystemAgentAction->isChecked()){
#ifdef QT_DEBUG
                qDebug()<<"重新启动";
#endif
                controller->stop();
                if (!configs.isEmpty()){
                    Profile& profile=configs.first().profile;
                    controller->setup(profile);
                    localAddress=profile.local_address;
                    localPort=QString::number(profile.local_port);
                    controller->start();
                }
            }
        }
    });
    connect(exitAction, &QAction::triggered, []() {
        qApp->exit();
    });


    updateServerMenu();
}

void SystemTrayIcon::setProxyMethod(QString proxyMethod) {
    QDBusPendingCallWatcher *w = new QDBusPendingCallWatcher(networkInter.SetProxyMethod(proxyMethod), this);
    QObject::connect(w, &QDBusPendingCallWatcher::finished, [=] {
        qDebug()<<"success to set proxy method "<<proxyMethod;
    });
    connect(w, &QDBusPendingCallWatcher::finished, w, &QDBusPendingCallWatcher::deleteLater);
}

void SystemTrayIcon::setManualProxy() {
    setProxyMethod("manual");
    QString type="socks";
//    QString addr="127.0.0.1";
    QString addr=localAddress;
//    QString port="1080";
    QString port=localPort;
    QDBusPendingCallWatcher *w = new QDBusPendingCallWatcher(networkInter.SetProxy(type, addr, port), this);
    QObject::connect(w, &QDBusPendingCallWatcher::finished, [=] {
        qDebug()<<"set proxy"<<type<<addr<<port;
    });
}

void SystemTrayIcon::setAutoProxy() {
    setProxyMethod("auto");
    // 目前先写死吧
    QString proxy="file:///etc/ss/autoproxy.pac";
    QDBusPendingCallWatcher *w = new QDBusPendingCallWatcher(networkInter.SetAutoProxy(proxy), this);

    QObject::connect(w, &QDBusPendingCallWatcher::finished, [](){
        qDebug()<<"finished";
    });
}

void SystemTrayIcon::updateServerMenu() {
#ifdef QT_DEBUG
    qDebug()<<"刷新服务器列表";
#endif
    serverMenu->clear();
    serverMenu->addAction(loadBalancingAction);
    serverMenu->addAction(highAvailabilityAction);
    serverMenu->addAction(accordingToStatisticsAction);
    serverMenu->addSeparator();
    if(!configs.isEmpty()){
        delete serverGroup;
        QAction* t;
        serverGroup=new QActionGroup(this);
        connect(serverGroup,&QActionGroup::triggered,this,&SystemTrayIcon::onServerActionTriggered);
        for (int i = 0; i < configs.size(); ++i) {
            auto it=configs[i];
#ifdef QT_DEBUG
            qDebug()<<it.remarks;
#endif
            ServerAction* action=new ServerAction(it.remarks,this);
            action->profile=it.profile;
            action->setCheckable(true);
            serverGroup->addAction(action);
            serverMenu->addAction(action);
            if(i==0){
                t=action;
            }
#ifdef QT_DEBUG
            output(action->profile);
#endif
        }
        serverMenu->addSeparator();
        t->trigger();
    } else{
        startSystemAgentAction->setEnabled(false);
    }
    serverMenu->addAction(editServerAction);
    serverMenu->addAction(statisticsConfigurationAction);
    serverMenu->addSeparator();
    serverMenu->addAction(shareServerConfigurationAction);
    serverMenu->addAction(scanThe2DCodeOnTheScreenAction);
    serverMenu->addAction(importTheURLFromTheClipboardAction);

}

void SystemTrayIcon::initConfig() {
    configs=ConfigUtil::readConfig();
}

void SystemTrayIcon::onServerActionTriggered(QAction *action) {
    ServerAction* serverAction= dynamic_cast<ServerAction *>(action);
    controller->setup(serverAction->profile);
    localAddress=serverAction->profile.local_address;
    localPort=QString::number(serverAction->profile.local_port);
    if(startSystemAgentAction->isChecked()){
        controller->stop();
        controller->start();
    }
}

ServerAction::ServerAction(const QString &text, QObject *parent) : QAction(text, parent) {

}
