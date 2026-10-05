import CardManager from "@/components/CardManager";
import CameraViewer from "@/components/CameraViewer";

export default function Home() {
    return (
        <div className="flex flex-col flex-1 items-center justify-center bg-zinc-50 font-sans dark:bg-black">
            <main className="flex flex-1 w-full max-w-3xl flex-col items-center justify-center gap-10 py-16 px-8 bg-white dark:bg-black">
                <CardManager />
                <CameraViewer />
            </main>
        </div>
    );
}