package com.lacina.cubeeclient.utils;


import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Point;
import android.location.LocationManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.provider.Settings;
import android.util.Log;
import android.view.Display;
import android.view.Gravity;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Chronometer;
import android.widget.EditText;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.text.Normalizer;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Collections;
import java.util.Enumeration;
import java.util.List;
import java.util.Locale;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;


@SuppressLint("NewApi")
public abstract class ActivityUtils {
	private static File externalStorage = Environment.getExternalStorageDirectory();
	private static String externalPath = externalStorage.getPath();
	private static ProgressDialog progressDialog;
	private static AlertDialog.Builder alertDialogSairSemSalvar;
	private static AlertDialog.Builder alertDialogExcluir;
	private static Activity callingActivity;
	private static String TAG = "ActivityUtils";
	private static int t = 5;
	private static Toast toast;
	private static Chronometer chronometer;
	private static boolean isToasted = false;


	//date
	public static String getStringCurrentDate(){
		Calendar c = Calendar.getInstance();
		SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.ENGLISH);
		return sdf.format(c.getTime());
	}

	public static void delay(int seconds){
		final Handler handler = new Handler();
		handler.postDelayed(new Runnable() {
			@Override
			public void run() {
				// Do something after 5s = 5000ms

			}
		}, seconds * 1000);
	}

	public static Bitmap rotateMyBitmap(Bitmap source, float angle){
		Matrix matrix = new Matrix();
		matrix.postRotate(angle);
		return Bitmap.createBitmap(source, 0, 0, source.getWidth(), source.getHeight(), matrix, true);
	}

	public static Double trataRotacao(Double rotacao, int regulador) {
		if (rotacao + regulador >= 360) {
			rotacao -= 360;
		}
		return rotacao;
	}

	public static Integer trataRotacao(Integer rotacao, int regulador) {
		if (rotacao + regulador >= 360) {
			rotacao -= 360;
		}
		return rotacao;
	}

	public static Double trataRotacao2(Double rotacao) {
		if(rotacao<0){
			rotacao = 360 + rotacao;
		}

		return rotacao;
	}

	// Toast
	public static boolean isToasted(){
		return isToasted;
	}

	public static void setToastedStatus(boolean status){
		isToasted = status;
	}


	public static Toast getToast() {
		return toast;
	}

	public static void cancelToast(){
		if(toast != null){
			toast.cancel();
		}
	}

	public void showToastDefinindoTempo(Context context, String text, int duration){
		final Toast toast = Toast.makeText(context, text, Toast.LENGTH_SHORT);
		toast.show();
		Handler h = new Handler();
		h.postDelayed(new Runnable() {
			@Override
			public void run() {
				toast.cancel();
			}
		}, duration);
	}

	public static void showToast(Context context, String text) {
		toast = Toast.makeText(context, text, Toast.LENGTH_SHORT);
		toast.setGravity(Gravity.TOP|Gravity.CENTER, 0, 20);
		toast.show();
	}

	public static void showToastLong(Context context, String text) {
		toast = Toast.makeText(context, text, Toast.LENGTH_LONG);
		toast.setGravity(Gravity.TOP|Gravity.CENTER, 0, 20);
		toast.show();
	}


	public static void showToastNoInferiorDaTela(Context context, String text) {
		toast = Toast.makeText(context, text, Toast.LENGTH_SHORT);
		toast.show();
	}

	public static void showToastLongNoSuperiorDaTela(Context context, String text) {
		toast = Toast.makeText(context, text, Toast.LENGTH_LONG);
		toast.setGravity(Gravity.TOP|Gravity.CENTER, 0, 20);
		toast.show();
	}

	public static boolean isToastVisible() {
		boolean retorno = false;
		if (toast != null && toast.getView().isShown()){
			retorno = true;
		}
		return retorno;
	}

	public static void esperarToast() {
		Handler handler = new Handler();
		handler.postDelayed(new Runnable() {
			public void run() {
				isToasted = false;
			}
		}, 2000);
	}

	public static void showToastOutActivity(final Context context,	final String text) {
		Handler handler = new Handler(Looper.getMainLooper());
		handler.post(new Runnable() {

			@Override
			public void run() {
				Toast.makeText(context, text, Toast.LENGTH_LONG).show();

			}
		});
	}
	//ProgressDialog
	public static void showProgressDialogOutActivity(final Activity activity,	final String text) {
		Handler handler = new Handler(Looper.getMainLooper());
		handler.post(new Runnable() {
			@Override
			public void run() {
				showProgressDialog(activity, text);

			}
		});
	}

	public static void showProgressDialog(Activity activity, String message) {

		progressDialog = new ProgressDialog(activity);
		progressDialog.setMessage(message);
		progressDialog.setCanceledOnTouchOutside(false);

		progressDialog.show();
	}

	public static void cancelProgressDialog() {

		if(progressDialog != null){
			progressDialog.cancel();
		}

	}

	public static String getSSID(Context context){
		String ssid  = null;
		ConnectivityManager connManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
		NetworkInfo networkInfo = connManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
		if (networkInfo.isConnected()) {
			WifiManager wifiManager = (WifiManager) context.getApplicationContext().getSystemService(Context.WIFI_SERVICE);
			WifiInfo info = wifiManager.getConnectionInfo();
			ssid = info.getSSID();
			ssid = ssid.substring(1, ssid.length()-1);
		}
		return ssid;
	}

	public static ProgressDialog getProgreesDialog() {
		return progressDialog;
	}

	// AlertDialog
	public static void alertDialogSimple(final Activity activity, String title, String message ) {
		Context context = activity.getApplicationContext();
		AlertDialog.Builder alertDialogExitWithoutSave = new AlertDialog.Builder(
				activity);
		// Setting Dialog Title
		alertDialogExitWithoutSave.setTitle(title);

		// Setting Dialog Message
		alertDialogExitWithoutSave
				.setMessage(message);

		// Setting Icon to Dialog
		// alertDialogExitWithoutSave.setIcon(R.drawable.alerta);

		// Setting Negative "NO" Button
		alertDialogExitWithoutSave.setNegativeButton("OK!",
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						// Faz nada
						dialog.cancel();
					}
				});

		// Showing Alert Message
		alertDialogExitWithoutSave.show();
	}

	// AlertDialog
	public static void alertDialogSimpleCallback(final Activity activity, String title, String message, final DialogInterface.OnClickListener onClickListener) {
		Context context = activity.getApplicationContext();
		AlertDialog.Builder alertDialogExitWithoutSave = new AlertDialog.Builder(
				activity);
		// Setting Dialog Title
		alertDialogExitWithoutSave.setTitle(title);

		// Setting Dialog Message
		alertDialogExitWithoutSave
				.setMessage(message);

		// Setting Icon to Dialog
		// alertDialogExitWithoutSave.setIcon(R.drawable.alerta);

		// Setting Negative "NO" Button
		alertDialogExitWithoutSave.setNegativeButton("OK!", null);

		alertDialogExitWithoutSave.setOnDismissListener(new DialogInterface.OnDismissListener() {
			@Override
			public void onDismiss(DialogInterface dialog) {
				onClickListener.onClick(dialog, 0);
			}
		});

		// Showing Alert Message
		alertDialogExitWithoutSave.show();
	}

    // AlertDialog
    public static void alertDialogSimpleCallbackBtnMessages(final Activity activity, String title, String message, String nameBtnNegative, String nameBtnPositive, final DialogInterface.OnClickListener onClickListener) {
        Context context = activity.getApplicationContext();
        final AlertDialog.Builder alertDialogExitWithoutSave = new AlertDialog.Builder(
                activity);
        // Setting Dialog Title
        alertDialogExitWithoutSave.setTitle(title);

        // Setting Dialog Message
        alertDialogExitWithoutSave
                .setMessage(message);

        // Setting Icon to Dialog
        // alertDialogExitWithoutSave.setIcon(R.drawable.alerta);

        // Setting Negative "NO" Button
        alertDialogExitWithoutSave.setNegativeButton(nameBtnNegative, null);
        alertDialogExitWithoutSave.setPositiveButton(nameBtnPositive, onClickListener);


        alertDialogExitWithoutSave.setOnDismissListener(new DialogInterface.OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                dialog.cancel();
            }
        });

        // Showing Alert Message
        alertDialogExitWithoutSave.show();
    }

	public static void alertDialogEnableGPS(final Context context, final Activity activity ) {

		String titulo = "GPS desativado";
		String mensagem = "Por favor, ative o GPS.";

		AlertDialog.Builder alertDialogSairSemSalvar = new AlertDialog.Builder(
				activity);
		alertDialogSairSemSalvar.setCancelable(false);
		alertDialogSairSemSalvar.setTitle(titulo);
		alertDialogSairSemSalvar.setMessage(mensagem);
		alertDialogSairSemSalvar.setPositiveButton("Cancelar",
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						dialog.cancel();
					}
				});

		alertDialogSairSemSalvar.setNegativeButton("OK",
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						// Faz nada

						Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
						activity.startActivity(intent);
					}
				});

		// Showing Alert Message
		alertDialogSairSemSalvar.show();
	}

	public static void alertDialogSimpleDualButton(final Activity activity, String titulo, String mensagem,
											 DialogInterface.OnClickListener clickListenerSuccess) {


		AlertDialog.Builder alertDialogSairSemSalvar = new AlertDialog.Builder(
				activity);
		alertDialogSairSemSalvar.setTitle(titulo);
		alertDialogSairSemSalvar.setMessage(mensagem);
		alertDialogSairSemSalvar.setPositiveButton("Cancelar",
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						dialog.cancel();
					}
				});

		alertDialogSairSemSalvar.setNegativeButton("OK", clickListenerSuccess);

		// Showing Alert Message
		alertDialogSairSemSalvar.show();
	}

	public static void alertDialogSimpleDualButton(final Activity activity, String titulo, String mensagem,
												   DialogInterface.OnClickListener clickListenerSuccess,
												   DialogInterface.OnClickListener clickListenerCancel) {


		AlertDialog.Builder alertDialogSairSemSalvar = new AlertDialog.Builder(
				activity);
		alertDialogSairSemSalvar.setTitle(titulo);
		alertDialogSairSemSalvar.setMessage(mensagem);
		alertDialogSairSemSalvar.setPositiveButton("Cancelar",clickListenerCancel);

		alertDialogSairSemSalvar.setNegativeButton("OK", clickListenerSuccess);

		// Showing Alert Message
		alertDialogSairSemSalvar.show();
	}

	public static void alertDialogEnableInternet(final Context context, final Activity activity ) {

		String titulo = "Internet desativada";
		String mensagem = "Por favor, ative a internet.";



		AlertDialog.Builder alertDialogSairSemSalvar = new AlertDialog.Builder(
				activity);
		alertDialogSairSemSalvar.setCancelable(false);
		alertDialogSairSemSalvar.setTitle(titulo);
		alertDialogSairSemSalvar.setMessage(mensagem);
		alertDialogSairSemSalvar.setPositiveButton("Wifi",
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						activity.startActivity(new Intent(Settings.ACTION_WIFI_SETTINGS));
					}
				});

		alertDialogSairSemSalvar.setNegativeButton("Rede móvel",
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						activity.startActivity(new Intent(Settings.ACTION_WIRELESS_SETTINGS));

					}
				});

		// Showing Alert Message
		alertDialogSairSemSalvar.show();
	}

	public static void alertDialogEditText(final Activity activity, final String titulo, final String mensagem){
		final Context context = activity.getApplicationContext();
		final AlertDialog.Builder inputAlert = new AlertDialog.Builder(activity);
		inputAlert.setTitle(titulo);
		inputAlert.setMessage(mensagem);
		final EditText userInput = new EditText(activity);
		inputAlert.setView(userInput);
		inputAlert.setPositiveButton("OK", new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				String userInputValue = userInput.getText().toString().trim();
				//TODO Adicione a ação desejada aqui



			}
		});
		AlertDialog alertDialog = inputAlert.create();
		alertDialog.show();
	}


	public static void alertDialogEditTextCallback(final Activity activity, final String titulo, final String mensagem, final DialogInterface.OnClickListener onClickListener){
		final Context context = activity.getApplicationContext();
		final AlertDialog.Builder inputAlert = new AlertDialog.Builder(activity);
		inputAlert.setTitle(titulo);
		inputAlert.setMessage(mensagem);
		final EditText userInput = new EditText(activity);
		inputAlert.setView(userInput);
		inputAlert.setPositiveButton("OK", new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				String userInputValue = userInput.getText().toString().trim();
				onClickListener.onClick(dialog, 0);
			}
		});
		AlertDialog alertDialog = inputAlert.create();
		alertDialog.show();
	}

	//Internet e GPS
	public static boolean isInternetEnable(Activity activity) {
		ConnectivityManager connectivity = (ConnectivityManager) activity
				.getSystemService(Context.CONNECTIVITY_SERVICE);
		if (connectivity.getActiveNetworkInfo() != null) {
			if (connectivity.getActiveNetworkInfo().isConnected())
				return true;
		}
		return false;
	}

	public static boolean isGPSEnable(Activity activity) {
		LocationManager service = (LocationManager) activity.getSystemService(activity.LOCATION_SERVICE);

		return service.isProviderEnabled(LocationManager.GPS_PROVIDER);
	}

	//------------------------------------------------------------------------------------------



	public static void initChronometer(Context context){
		chronometer = new Chronometer(context);
	}
	public static void deactivateChronometer(Context context){
		chronometer = null;
	}
	public static void startChronometer(){
		chronometer.setBase(SystemClock.elapsedRealtime());
		chronometer.start();
	}
	public static  void stopChronometer(){
		chronometer.stop();
	}
	public static  Chronometer getChronometer(){
		return chronometer;
	}

	public static Long getChronometerTime(){
		Long elapsedMillis = null;
		if(chronometer!= null){
			elapsedMillis = SystemClock.elapsedRealtime() - chronometer.getBase();
		}
		return elapsedMillis;
	}

	public static int calcDPI(Activity activity) {
		float scale = activity.getResources().getDisplayMetrics().density;
		return (int) (1 * scale + 0.5f);
	}

	public static int calcDPI(View activity) {
		float scale = activity.getResources().getDisplayMetrics().density;
		return (int) (1 * scale + 0.5f);
	}

	public static String removeAccents(String string) {
		char[] out = new char[string.length()];
		string = Normalizer.normalize(string, Normalizer.Form.NFD);
		int j = 0;
		for (int i = 0, n = string.length(); i < n; ++i) {
			char c = string.charAt(i);
			if (c <= '\u007F')
				out[j++] = c;
		}
		return new String(out);
	}

	public static void createDirectoryOnSdCard(String nameFolder) {
		File newFolder = new File(nameFolder);

		boolean exist = newFolder.exists();

		if (!exist) {
			newFolder.mkdir();
		}
	}

	public static boolean deleteDir(File dir) {
		if (dir != null && dir.isDirectory()) {
			String[] children = dir.list();
			for (int i = 0; i < children.length; i++) {
				boolean success = deleteDir(new File(dir, children[i]));
				if (!success) {
					return false;
				}
			}
		}
		return dir.delete();
	}

	public static boolean deleteDir(String pathDir) {
		File externalStorage = Environment.getExternalStorageDirectory();
		String externalPath = externalStorage.getPath();
		File dir = new File(externalPath + "/" + pathDir);
		if (dir != null && dir.isDirectory()) {
			String[] children = dir.list();
			for (int i = 0; i < children.length; i++) {
				boolean success = deleteDir(new File(dir, children[i]));
				if (!success) {
					return false;
				}
			}
		}
		return dir.delete();
	}

	public static void deleteArq(File arq) {
		if (arq != null) {
			arq.delete();
		}
	}

	public static void deleteFotosFiles(File arq) {
		if (arq != null) {
			arq.delete();

			// Apaga a pasta caso esteja vazia
			if (arq.getParentFile().isDirectory()) {
				File[] contents = arq.getParentFile().listFiles();
				if (contents.length == 0) {
					arq.getParentFile().delete();
				}
			}
		}
	}

	public static boolean directoryIsEmpty(File file) {
		boolean retorno = false;
		if (file.isDirectory()) {
			String[] files = file.list();
			if (files.length == 0) {
				retorno = true;
			}
		}
		return retorno;

	}

	public static void extractFolder(ZipFile zip, String extractFolder) {
		try {
			// Open the zip file
			ZipFile zipFile = zip;
			Enumeration<?> enu = zipFile.entries();
			while (enu.hasMoreElements()) {
				ZipEntry zipEntry = (ZipEntry) enu.nextElement();

				String name = zipEntry.getName().replace("\\", "/");
				String pathExtract = externalPath + "/" + extractFolder + "/"
						+ zipEntry.getName().replace("\\", "/");
				long size = zipEntry.getSize();
				long compressedSize = zipEntry.getCompressedSize();
				System.out.printf(
						"name: %-20s | size: %6d | compressed size: %6d\n",
						name, size, compressedSize);

				// Do we need to create a directory ?
				File file = new File(name);
				if (name.endsWith("/")) {
					file.mkdirs();
					continue;
				}

				File parent = file.getParentFile();
				if (parent != null && !parent.exists()) {
					parent.mkdirs();
				}

				// Extract the file
				File f = new File(pathExtract);
				if (!f.exists()) {
					f.createNewFile();
				}
				InputStream is = zipFile.getInputStream(zipEntry);
				FileOutputStream fos = new FileOutputStream(f);
				byte[] bytes = new byte[1024];
				int length;
				while ((length = is.read(bytes)) >= 0) {
					fos.write(bytes, 0, length);
				}
				is.close();
				fos.close();

			}
			zipFile.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public static void setCallingActivity(Activity activity) {
		callingActivity = activity;
	}

	public static Activity getCallingActivity() {
		return callingActivity;
	}

	public static void colocaListaEmOrdemAlfabetica(List<String> lista) {
		Collections.sort(lista);
		lista.add(0, "Selecione");
	}

	public static void copyDirectory(File sourceLocation, File targetLocation)
			throws IOException {

		if (sourceLocation.isDirectory()) {
			if (!targetLocation.exists() && !targetLocation.mkdirs()) {
				throw new IOException("Cannot create dir "
						+ targetLocation.getAbsolutePath());
			}

			String[] children = sourceLocation.list();
			for (int i = 0; i < children.length; i++) {
				copyDirectory(new File(sourceLocation, children[i]), new File(
						targetLocation, children[i]));
			}
		} else {

			// make sure the directory we plan to store the recording in exists
			File directory = targetLocation.getParentFile();
			if (directory != null && !directory.exists() && !directory.mkdirs()) {
				throw new IOException("Cannot create dir "
						+ directory.getAbsolutePath());
			}

			InputStream in = new FileInputStream(sourceLocation);
			OutputStream out = new FileOutputStream(targetLocation);

			// Copy the bits from instream to outstream
			byte[] buf = new byte[1024];
			int len;
			while ((len = in.read(buf)) > 0) {
				out.write(buf, 0, len);
			}
			in.close();
			out.close();
		}
	}

	public static void deleteFilesAndSubFiles(File f) {
		String[] flist = f.list();
		for (int i = 0; i < flist.length; i++) {
			System.out.println(" " + f.getAbsolutePath());
			File temp = new File(f.getAbsolutePath() + "/" + flist[i]);
			if (temp.isDirectory()) {
				deleteFilesAndSubFiles(temp);
				temp.delete();
			} else {
				temp.delete();
			}
		}
	}

	public static void createDirectoryOnSdCardFirstTime(String nameFolder,	InputStream open) {
		File newFolder = new File( nameFolder);
		boolean exist = newFolder.exists();

		if (!exist) {
			newFolder.mkdir();
			inputStreanToFile(open, new File(externalPath + "/" + nameFolder + "/" + "programacao_campina.txt"));
			//TODO tranferir programa�oes  das outras cidades da pasta assests para a pasta do vumbora
		}
	}

	public static void inputStreanToFile(InputStream open, File file) {

		InputStream inputStream = null;
		OutputStream outputStream = null;

		try {
			// read this file into InputStream
			inputStream = open;

			// write the inputStream to a FileOutputStream
			outputStream = new FileOutputStream(file);

			int read = 0;
			byte[] bytes = new byte[1024];

			while ((read = inputStream.read(bytes)) != -1) {
				outputStream.write(bytes, 0, read);
			}

			System.out.println("Done!");

		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			if (inputStream != null) {
				try {
					inputStream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			if (outputStream != null) {
				try {
					// outputStream.flush();
					outputStream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}

			}
		}
	}

	public static void setStatusBarColor(Activity activity, String hexadecimalColor){
		int col = Color.parseColor(hexadecimalColor);
		if (Build.VERSION.SDK_INT >= 21) {
			Window window = activity.getWindow();
			window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
			window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
			window.setStatusBarColor(col);
		}
	}

	public static double getScreenHeight(Activity activity){
		Display display = activity.getWindowManager().getDefaultDisplay();
		Point size = new Point();
		display.getSize(size);

		int height = size.y;
		return height;
	}

	public static String getCurrentDate(){
		Calendar c = Calendar.getInstance();
		Log.i(TAG, c.getTime() + "");
		String DATE_FORMAT = "dd-MM-yyyy HH:mm:ss.SSS";
		SimpleDateFormat sdf = new SimpleDateFormat();

		return sdf.format(c.getTime());


		//SimpleDateFormat df = new SimpleDateFormat("dd-MMM-yyyy");
		//String formattedDate = df.format(c.getTime());
	}

	public static String convertInputStreamToString(InputStream is) {
		java.util.Scanner s = new java.util.Scanner(is).useDelimiter("\\A");
		return s.hasNext() ? s.next() : "";
	}

	public static String encodeURL(String url) {
		String encodedURL = "";
		try {
			encodedURL = URLEncoder.encode(url, "utf-8");
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		}
		return encodedURL;
	}

	private String convertStreamToString(final InputStream input) throws Exception {
		try {
			final BufferedReader reader = new BufferedReader(new InputStreamReader(input));
			final StringBuffer sBuf = new StringBuffer();
			String line = null;
			while ((line = reader.readLine()) != null) {
				sBuf.append(line);
			}
			return sBuf.toString();
		} catch (Exception e) {
			throw e;
		} finally {
			try {
				input.close();
			} catch (Exception e) {
				throw e;
			}
		}
	}
}
