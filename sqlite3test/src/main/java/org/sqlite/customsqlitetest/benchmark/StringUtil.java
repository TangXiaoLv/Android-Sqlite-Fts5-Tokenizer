package org.sqlite.customsqlitetest.benchmark;

import java.util.Random;

public class StringUtil {

    private static final String TABLE = "昆仑山地方就居然被啊收到了付款就开车v你" +
            "lkjdfgsf来的客服经理sdf恶恶然后hkhlbbaoew删了可地方尽快拿出，" +
            "打发莫妮卡的好看迪卡侬，模拟参加考试的话付款计划客家话" +
            "的旧方法轮廓设计符合ioef，不能每次v了很快‘；了对方考虑【乐然里5" +
            "546654576u地方vu分别走访了asdkfghdugknmbcvkesrt" +
            "可能吧看龙剑打飞机了Kerr托v家0580【中长距离课程bfhgsir" +
            "dgklajrlt风格化反馈嘎哈ui人会认为分库分那边v看fgklsjeto" +
            "地方v奶茶铺i皮特人品trot软件哦然后刚好，从v你，bnhghjllks" +
            "开发的感觉到了法国尽量让他赶紧离开雷锋ljiowe";

    private static final int LEN = TABLE.length();

    private static Random random = new Random();

    public static String randomString() {
        int scope = random.nextInt(LEN);
        if (scope == 0) {
            return randomString();
        }
        StringBuilder sb = new StringBuilder(scope);
        for (int i = 0; i < scope; i++) {
            int index = random.nextInt(LEN);
            sb.append(TABLE.charAt(index));
        }
        return sb.toString();
    }
}
