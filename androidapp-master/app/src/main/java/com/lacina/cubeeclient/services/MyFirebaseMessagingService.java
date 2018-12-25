package com.lacina.cubeeclient.services;


import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.media.RingtoneManager;
import android.net.Uri;
import android.support.v4.app.NotificationCompat;
import android.util.Log;

import com.google.firebase.messaging.FirebaseMessagingService;
import com.google.firebase.messaging.RemoteMessage;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.activities.MainActivity;
import com.lacina.cubeeclient.utils.NotificationEvent;

import org.greenrobot.eventbus.EventBus;

@SuppressWarnings("ALL")
public class MyFirebaseMessagingService extends FirebaseMessagingService {

    private static final String TAG = "MyFirebaseMsgService";

    @Override
    public void onMessageReceived(RemoteMessage remoteMessage) {
        Log.d(TAG, "From: " + remoteMessage.getFrom());

//        if (remoteMessage.getData().size() > 0) {
//            Log.d(TAG, "Message data payload: " + remoteMessage.getData());
//                handleNow(remoteMessage);
//        }

        // Check if message contains a notification payload.
        if (remoteMessage.getNotification() != null) {
            handleNow(remoteMessage);
        }

    }



    private void handleNow(RemoteMessage remoteMessage) {
        Log.d(TAG, "Message data payload: " + remoteMessage.getData());
        String body = remoteMessage.getNotification().getBody();
        EventBus.getDefault().post(new NotificationEvent(body));

    }

    /**
     * Create and show a simple notification containing the received FCM message.
     *
     * @param messageBody FCM message body received.
     */
    @SuppressWarnings("unused")
    private void sendNotification(String messageBody) {
        Intent intent = new Intent(this, MainActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0 /* Request code */, intent,
                PendingIntent.FLAG_ONE_SHOT);

        Uri defaultSoundUri= RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);
        NotificationCompat.Builder notificationBuilder = new NotificationCompat.Builder(this)
                .setSmallIcon(R.mipmap.cubee_icon)
                .setContentTitle("FCM Message")
                .setContentText(messageBody)
                .setAutoCancel(true)
                .setSound(defaultSoundUri)
                .setContentIntent(pendingIntent);

        NotificationManager notificationManager =
                (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);

        notificationManager.notify(0 /* ID of notification */, notificationBuilder.build());
    }
}